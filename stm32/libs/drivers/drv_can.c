#include "drv_can.h"

drc_can_t *drc_can_list[2] = {0, 0};

bool drv_can_pin_source_init(uint8_t num, uint8_t tx_selec, uint8_t rx_selec)
{
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
        drv_gpio_init(tx_pin->port, tx_pin->pin_num, IOMODE_AFPP,
						IO_PULLUP, IO_SPEEDHIGH, tx_pin->alternate, NULL);
        drv_gpio_init(rx_pin->port, rx_pin->pin_num, GPIO_MODE_AF_PP,
						IO_PULLUP, IO_SPEEDHIGH, rx_pin->alternate, NULL);
	} else {
		return false;
	}
	return true;
}

void drv_can_attr_init(struct drv_can_attr *obj, uint32_t baud, uint16_t priority,
                    struct drv_canframe *array, uint16_t len)
{
    obj->baud = baud;
    obj->priority = priority;
    obj->array = array;
    obj->capacity = len;
    obj->size = obj->in = obj->out = 0;
}

void drv_can_init(struct drc_can_t *obj, uint8_t num, uint8_t tx_selec, 
                    uint8_t rx_selec, struct drv_can_attr *attr)
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

    if (!drv_can_pin_source_init(num, tx_selec, rx_selec)) return;
    obj->attr = *attr;

    uint16_t presc = 10;
    if (attr->baud  == 1000) 	presc = 5;	
    if (attr->baud  == 500) 	presc = 10;	
    if (attr->baud  == 250) 	presc = 20;
    if (attr->baud  == 200) 	presc = 25;
    if (attr->baud  == 100) 	presc = 50;
    HAL_FDCAN_DeInit(&obj->hcan);
    HAL_NVIC_DisableIRQ(can_irq[num-1]);

    obj->hcan.Instance = can_instance[num-1];
    obj->hcan.Init.FrameFormat = FDCAN_FRAME_CLASSIC;
    obj->hcan.Init.Mode = FDCAN_MODE_NORMAL;
    obj->hcan.Init.AutoRetransmission = DISABLE;
    obj->hcan.Init.TransmitPause = DISABLE;
    obj->hcan.Init.ProtocolException = DISABLE;
    obj->hcan.Init.NominalPrescaler = presc;
    obj->hcan.Init.NominalSyncJumpWidth = 8;
    obj->hcan.Init.NominalTimeSeg1 = 31;
    obj->hcan.Init.NominalTimeSeg2 = 8;
    obj->hcan.Init.MessageRAMOffset = can_memoffset[num-1];
    obj->hcan.Init.StdFiltersNbr = 1;
    obj->hcan.Init.ExtFiltersNbr = 0;
    obj->hcan.Init.RxFifo0ElmtsNbr = 2;
    obj->hcan.Init.RxFifo0ElmtSize = FDCAN_DATA_BYTES_8;
    obj->hcan.Init.RxFifo1ElmtsNbr = 0;
    obj->hcan.Init.RxFifo1ElmtSize = FDCAN_DATA_BYTES_8;
    obj->hcan.Init.RxBuffersNbr = 0;
    obj->hcan.Init.TxEventsNbr =	0;
    obj->hcan.Init.TxBuffersNbr = 0;
    obj->hcan.Init.TxFifoQueueElmtsNbr = 2;
    obj->hcan.Init.TxFifoQueueMode = FDCAN_TX_FIFO_OPERATION;
    obj->hcan.Init.TxElmtSize =	FDCAN_DATA_BYTES_8;
    HAL_FDCAN_Init(&obj->hcan);

    FDCAN_FilterTypeDef CANx_RxFilter = {0};
    CANx_RxFilter.IdType = FDCAN_STANDARD_ID;
    CANx_RxFilter.FilterIndex = 0;
    CANx_RxFilter.FilterType = FDCAN_FILTER_MASK;
    CANx_RxFilter.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;
    CANx_RxFilter.FilterID1 = 0x0000;
    CANx_RxFilter.FilterID2 = 0x0000;
    HAL_FDCAN_ConfigFilter(&obj->hcan, &CANx_RxFilter);
    HAL_FDCAN_Start(&obj->hcan);

    HAL_FDCAN_ActivateNotification(&obj->hcan, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0);
    HAL_NVIC_SetPriority(can_irq[num-1], obj->attr.priority, 0);
    HAL_NVIC_EnableIRQ(can_irq[num-1]);

	drc_can_list[num-1] = obj;
}

void drv_can_send(struct drc_can_t *obj, struct drv_canframe *frame)
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

    HAL_FDCAN_AddMessageToTxFifoQ(&obj->hcan, &lowlevel_msg, &frame->_data[0]);
}

bool drv_can_get(struct drc_can_t *obj, struct drv_canframe *frame)
{
	if (obj->attr.size <= 0) return false;

	*frame = obj->attr.array[obj->attr.out];
	obj->attr.out++;
	if (obj->attr.out >= obj->attr.capacity) {
        obj->attr.out -= obj->attr.capacity;
	}
    obj->attr.size--;
}

void drv_can_rx_irq(struct drc_can_t *obj, uint32_t fifox)
{
	FDCAN_RxHeaderTypeDef msg;
	struct drv_canframe frame;

    if (obj == NULL) return;

    HAL_FDCAN_GetRxMessage(&obj->hcan, fifox, &msg, frame._data);
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

    if (obj->attr.size < obj->attr.capacity) {
        obj->attr.array[obj->attr.in] = frame;
        obj->attr.in++;
        if (obj->attr.in >= obj->attr.capacity) {
            obj->attr.in -= obj->attr.capacity;
        }
        obj->attr.size++;
	}
}


void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs)
{
	if ((RxFifo0ITs & FDCAN_IT_RX_FIFO0_NEW_MESSAGE) != RESET) {
		if (hfdcan->Instance == FDCAN1)
			drv_can_rx_irq(drc_can_list[0], FDCAN_RX_FIFO0);
		else if (hfdcan->Instance == FDCAN2)
			drv_can_rx_irq(drc_can_list[1], FDCAN_RX_FIFO0);
			
		HAL_FDCAN_ActivateNotification(hfdcan, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0);
	}
}

void HAL_FDCAN_RxFifo1Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo1ITs)
{
	if((RxFifo1ITs & FDCAN_IT_RX_FIFO1_NEW_MESSAGE) != RESET) {
		if (hfdcan->Instance == FDCAN1)
			drv_can_rx_irq(drc_can_list[0], FDCAN_RX_FIFO1);
		else if (hfdcan->Instance == FDCAN2)
			drv_can_rx_irq(drc_can_list[1], FDCAN_RX_FIFO1);
		
		HAL_FDCAN_ActivateNotification(hfdcan, FDCAN_IT_RX_FIFO1_NEW_MESSAGE, 0);
	}
}


void FDCAN1_IT0_IRQHandler(void)
{
    HAL_FDCAN_IRQHandler(&(drc_can_list[0]->hcan));  
}

void FDCAN1_IT1_IRQHandler(void)
{
    HAL_FDCAN_IRQHandler(&(drc_can_list[0]->hcan));   
}

void FDCAN2_IT0_IRQHandler(void)
{
    HAL_FDCAN_IRQHandler(&(drc_can_list[1]->hcan));
}

void FDCAN2_IT1_IRQHandler(void)
{
    HAL_FDCAN_IRQHandler(&(drc_can_list[1]->hcan)); 
}
