#include "drv_can.h"

static struct up_can_dev_s *can_list[2] = {0, 0};

struct low_canframe {
    uint32_t _identifier;
    uint32_t _idtype;
    uint32_t _frametype;
    uint32_t _datalength;
    uint8_t  _data[8];
};

struct low_canfifo {
    struct low_canframe *array;
    uint16_t capacity;
    uint16_t size;
    uint16_t in;
    uint16_t out;
};

static bool low_pinconfig(struct up_can_dev_s *dev);
static void low_rxirq(struct up_can_dev_s *dev, uint32_t fifox);

bool low_pinconfig(struct up_can_dev_s *dev)
{
    uint8_t num = dev->id;
    uint8_t tx_selec = dev->pin_tx;
    uint8_t rx_selec = dev->pin_rx;
	const struct pin_node *tx_pin, *rx_pin;
	uint32_t illegal;
	
	switch (num) {
	case 1:
		if (PINNODE(uint32_t)(CAN_PINCTRL_SOURCE(1, CAN_PIN_TX, tx_selec)) != NULL
			&& PINNODE(uint32_t)(CAN_PINCTRL_SOURCE(1, CAN_PIN_RX, rx_selec)) != NULL) {
			tx_pin = CAN_PINCTRL_SOURCE(1, CAN_PIN_TX, tx_selec);
			rx_pin = CAN_PINCTRL_SOURCE(1, CAN_PIN_RX, rx_selec);
			illegal = tx_pin->port && rx_pin->port;
		}else {
			return false;
		}
		break;
	case 2:
		if (PINNODE(uint32_t)(CAN_PINCTRL_SOURCE(2, CAN_PIN_TX, tx_selec)) != NULL
			&& PINNODE(uint32_t)(CAN_PINCTRL_SOURCE(2, CAN_PIN_RX, rx_selec)) != NULL) {
			tx_pin = CAN_PINCTRL_SOURCE(2, CAN_PIN_TX, tx_selec);
			rx_pin = CAN_PINCTRL_SOURCE(2, CAN_PIN_RX, rx_selec);
			illegal = tx_pin->port && rx_pin->port;
		}else {
			return false;
		}
		break;
	default: return false;
	}
	
	if (illegal != 0) {
        low_gpio_setup(tx_pin->port, tx_pin->pin, IOMODE_AFPP, IO_PULLUP, IO_SPEEDHIGH, tx_pin->alternate, NULL, 0);
		low_gpio_setup(rx_pin->port, rx_pin->pin, IOMODE_AFPP, IO_PULLUP, IO_SPEEDHIGH, rx_pin->alternate, NULL, 0);
	} else {
		return false;
	}
	return true;
}

/**	
 * 	Initialize CAN Bus, include GPIO, CAN, CAN Filter, and NVIC. 
 *	the can bus baudrate computational formula is --->
 *						can bus clockFreq				200000000 (200MHz)
 *	can bus baudrate  = -------------------------- = ------------------------ = (500KHz)
 *						(ntsg1 + ntsg2 + 1) * presc 	(31 + 8 + 1) * 10
*/
void low_can_init(struct up_can_dev_s *dev)
{
	RCC_PeriphCLKInitTypeDef FDCAN_PeriphClk = {0};
    uint32_t can_irq[2] = {FDCAN1_IT0_IRQn, FDCAN2_IT0_IRQn};
    CAN_TypeDef can_instance[2] = {FDCAN1, FDCAN2};
    uint32_t can_memoffset[2] = {0, 1280};

	FDCAN_PeriphClk.PeriphClockSelection = RCC_PERIPHCLK_FDCAN;
	FDCAN_PeriphClk.FdcanClockSelection = RCC_FDCANCLKSOURCE_PLL;

	HAL_RCCEx_PeriphCLKConfig(&FDCAN_PeriphClk);
    __HAL_RCC_FDCAN_FORCE_RESET();
    __HAL_RCC_FDCAN_RELEASE_RESET();
	__HAL_RCC_FDCAN_CLK_ENABLE();

    if (!low_pinconfig(dev)) return;

    uint16_t presc = 10;
    if (dev->baud  == 1000) presc = 5;	
    if (dev->baud  == 500) 	presc = 10;	
    if (dev->baud  == 250) 	presc = 20;
    if (dev->baud  == 200) 	presc = 25;
    if (dev->baud  == 100) 	presc = 50;
    HAL_FDCAN_DeInit(&dev->hcan);
    HAL_NVIC_DisableIRQ(can_irq[num-1]);

    dev->hcan.Instance = can_instance[num-1];
    dev->hcan.Init.FrameFormat = FDCAN_FRAME_CLASSIC;
    dev->hcan.Init.Mode = FDCAN_MODE_NORMAL;
    dev->hcan.Init.AutoRetransmission = DISABLE;
    dev->hcan.Init.TransmitPause = DISABLE;
    dev->hcan.Init.ProtocolException = DISABLE;
    dev->hcan.Init.NominalPrescaler = presc;
    dev->hcan.Init.NominalSyncJumpWidth = 8;
    dev->hcan.Init.NominalTimeSeg1 = 31;
    dev->hcan.Init.NominalTimeSeg2 = 8;
    dev->hcan.Init.MessageRAMOffset = can_memoffset[num-1];
    dev->hcan.Init.StdFiltersNbr = 1;
    dev->hcan.Init.ExtFiltersNbr = 0;
    dev->hcan.Init.RxFifo0ElmtsNbr = 2;
    dev->hcan.Init.RxFifo0ElmtSize = FDCAN_DATA_BYTES_8;
    dev->hcan.Init.RxFifo1ElmtsNbr = 0;
    dev->hcan.Init.RxFifo1ElmtSize = FDCAN_DATA_BYTES_8;
    dev->hcan.Init.RxBuffersNbr = 0;
    dev->hcan.Init.TxEventsNbr =	0;
    dev->hcan.Init.TxBuffersNbr = 0;
    dev->hcan.Init.TxFifoQueueElmtsNbr = 2;
    dev->hcan.Init.TxFifoQueueMode = FDCAN_TX_FIFO_OPERATION;
    dev->hcan.Init.TxElmtSize =	FDCAN_DATA_BYTES_8;
    HAL_FDCAN_Init(&dev->hcan);

    FDCAN_FilterTypeDef CANx_RxFilter = {0};
    CANx_RxFilter.IdType = FDCAN_STANDARD_ID;
    CANx_RxFilter.FilterIndex = 0;
    CANx_RxFilter.FilterType = FDCAN_FILTER_MASK;
    CANx_RxFilter.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;
    CANx_RxFilter.FilterID1 = 0x0000;
    CANx_RxFilter.FilterID2 = 0x0000;
    HAL_FDCAN_ConfigFilter(&dev->hcan, &CANx_RxFilter);
    HAL_FDCAN_Start(&dev->hcan);

    HAL_FDCAN_ActivateNotification(&dev->hcan, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0);
    HAL_NVIC_SetPriority(can_irq[num-1], dev->priority, 0);
    HAL_NVIC_EnableIRQ(can_irq[num-1]);

	can_list[num-1] = dev;
}

void low_can_send(struct up_can_dev_s *dev, struct low_canframe *frame)
{
    FDCAN_TxHeaderTypeDef lowlevel_msg;

    lowlevel_msg.Identifier     = frame->_identifier;
    lowlevel_msg.IdType         = frame->_idtype;
    lowlevel_msg.TxFrameType    = frame->_frametype;
    lowlevel_msg.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
    lowlevel_msg.BitRateSwitch		 = FDCAN_BRS_OFF;
    lowlevel_msg.FDFormat			 = FDCAN_CLASSIC_CAN;
    lowlevel_msg.TxEventFifoControl	 = FDCAN_NO_TX_EVENTS;
    lowlevel_msg.MessageMarker		 = 0;

    switch (frame->_datalength) {
    case 0: lowlevel_msg.DataLength = FDCAN_DLC_BYTES_0;  break;
    case 1: lowlevel_msg.DataLength = FDCAN_DLC_BYTES_1;  break;
    case 2: lowlevel_msg.DataLength = FDCAN_DLC_BYTES_2;  break;
    case 3: lowlevel_msg.DataLength = FDCAN_DLC_BYTES_3;  break;
    case 4: lowlevel_msg.DataLength = FDCAN_DLC_BYTES_4;  break;
    case 5: lowlevel_msg.DataLength = FDCAN_DLC_BYTES_5;  break;
    case 6: lowlevel_msg.DataLength = FDCAN_DLC_BYTES_6;  break;
    case 7: lowlevel_msg.DataLength = FDCAN_DLC_BYTES_7;  break;
    case 8: lowlevel_msg.DataLength = FDCAN_DLC_BYTES_8;  break;
    }

    HAL_FDCAN_AddMessageToTxFifoQ(&dev->hcan, &lowlevel_msg, &frame->_data[0]);
}

bool low_can_recv(struct up_can_dev_s *dev, struct low_canframe *frame)
{
    struct low_canfifo *fifo = dev->fifo;
	if (fifo->size <= 0) return false;

	*frame = fifo->array[fifo->out];
	fifo->out++;
	if (fifo->out >= fifo->capacity) {
        fifo->out -= fifo->capacity;
	}
    fifo->size--;
    return true;
}

void low_rxirq(struct up_can_dev_s *dev, uint32_t fifox)
{
	FDCAN_RxHeaderTypeDef msg;
	struct drv_canframe frame;
    struct low_canfifo *fifo = dev->fifo;

    if (dev == NULL) return;

    HAL_FDCAN_GetRxMessage(&dev->hcan, fifox, &msg, frame._data);
	frame._identifier = msg.Identifier;
    frame._idtype = msg.IdType;
    frame._frametype = msg.RxFrameType;
	switch (msg.DataLength) {
	case FDCAN_DLC_BYTES_0: frame._datalength = 0; break;
	case FDCAN_DLC_BYTES_1: frame._datalength = 1; break;
	case FDCAN_DLC_BYTES_2: frame._datalength = 2; break;
	case FDCAN_DLC_BYTES_3: frame._datalength = 3; break;
	case FDCAN_DLC_BYTES_4: frame._datalength = 4; break;
	case FDCAN_DLC_BYTES_5: frame._datalength = 5; break;
	case FDCAN_DLC_BYTES_6: frame._datalength = 6; break;
	case FDCAN_DLC_BYTES_7: frame._datalength = 7; break;
	case FDCAN_DLC_BYTES_8: frame._datalength = 8; break;
	}

    if (fifo->size < fifo->capacity) {
        fifo->array[fifo->in] = frame;
        fifo->in++;
        if (fifo->in >= fifo->capacity) {
            fifo->in -= fifo->capacity;
        }
        fifo->size++;
	}
}

/****************************************************************************
 * STM32 HAL Library Callback 
 ****************************************************************************/
void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs)
{
	if ((RxFifo0ITs & FDCAN_IT_RX_FIFO0_NEW_MESSAGE) != RESET) {
		if (hfdcan->Instance == FDCAN1)
			drv_can_rx_irq(can_list[0], FDCAN_RX_FIFO0);
		else if (hfdcan->Instance == FDCAN2)
			drv_can_rx_irq(can_list[1], FDCAN_RX_FIFO0);
			
		HAL_FDCAN_ActivateNotification(hfdcan, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0);
	}
}

void HAL_FDCAN_RxFifo1Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo1ITs)
{
	if((RxFifo1ITs & FDCAN_IT_RX_FIFO1_NEW_MESSAGE) != RESET) {
		if (hfdcan->Instance == FDCAN1)
			drv_can_rx_irq(can_list[0], FDCAN_RX_FIFO1);
		else if (hfdcan->Instance == FDCAN2)
			drv_can_rx_irq(can_list[1], FDCAN_RX_FIFO1);
		
		HAL_FDCAN_ActivateNotification(hfdcan, FDCAN_IT_RX_FIFO1_NEW_MESSAGE, 0);
	}
}

void FDCAN1_IT0_IRQHandler(void)
{
    HAL_FDCAN_IRQHandler(&(can_list[0]->hcan));  
}

void FDCAN1_IT1_IRQHandler(void)
{
    HAL_FDCAN_IRQHandler(&(can_list[0]->hcan));   
}

void FDCAN2_IT0_IRQHandler(void)
{
    HAL_FDCAN_IRQHandler(&(can_list[1]->hcan));
}

void FDCAN2_IT1_IRQHandler(void)
{
    HAL_FDCAN_IRQHandler(&(can_list[1]->hcan)); 
}
