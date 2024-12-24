/**
 * low level driver for stm32h7 series, base on cubehal library
 * module can
*/

#include "include/lld_can.h"

lld_can_t *mcu_can_list[2] = {0, 0};

void lld_canframe_set(lld_canframe_t *obj, uint32_t id, uint32_t idtype, 
    uint32_t frametype, uint32_t len, uint8_t *data)
{
    obj->_identifier = id;
    obj->_idtype = idtype;
    obj->_frametype = frametype;
    obj->_datalength = len;
    for (uint8_t i = 0; i < len; i++)
        obj->_data[i] = data[i];
}

uint8_t lld_canframelist_size(lld_canframelist_t *obj) { return obj->size; }
void lld_canframelist_reset(lld_canframelist_t *obj)
{
    obj->idx_in = 0;
    obj->idx_out = 0;
    obj->size = 0;
}

void lld_canframelist_read(lld_canframelist_t *obj, lld_canframe_t *p)
{
    if (lld_canframelist_size(obj) == 0) return;
    
    lld_canframe_set(p, obj->buf[obj->idx_out]._identifier, 
    obj->buf[obj->idx_out]._idtype, obj->buf[obj->idx_out]._frametype,
    obj->buf[obj->idx_out]._datalength, obj->buf[obj->idx_out]._data);
    
    obj->idx_out++;
    if (obj->idx_out >= LLD_CANFRAMELIST_LEN) obj->idx_out = 0;
    obj->size--;
}

void lld_canframelist_write(lld_canframelist_t *obj, lld_canframe_t *p)
{
    if (lld_canframelist_size(obj) == LLD_CANFRAMELIST_LEN) return;
    
    lld_canframe_set(&obj->buf[obj->idx_in], p->_identifier, 
     p->_idtype, p->_frametype, p->_datalength, p->_data);
    
    obj->idx_in++;
    if (obj->idx_in >= LLD_CANFRAMELIST_LEN) obj->idx_in = 0;
    obj->size++;
}

bool lowlevel_can_pin_source_init(lld_can_t *obj, uint8_t can_num, uint8_t can_tx_selec, uint8_t can_rx_selec)
{
	const struct pin_node *can_tx_pin_node, *can_rx_pin_node;
	uint32_t is_pin_node_initable;
	
	switch (can_num) {
	case 1:
		if (PINNODE(uint32_t)(CAN_PINCTRL_SOURCE(1, CAN_PIN_TX, can_tx_selec)) != NULL
			&& PINNODE(uint32_t)(CAN_PINCTRL_SOURCE(1, CAN_PIN_RX, can_rx_selec)) != NULL) {
			can_tx_pin_node = CAN_PINCTRL_SOURCE(1, CAN_PIN_TX, can_tx_selec);
			can_rx_pin_node = CAN_PINCTRL_SOURCE(1, CAN_PIN_RX, can_rx_selec);
			is_pin_node_initable = can_tx_pin_node->gpio_port && can_rx_pin_node->gpio_port;
		}else {
			return false;
		}
		break;
	case 2:
		if (PINNODE(uint32_t)(CAN_PINCTRL_SOURCE(2, CAN_PIN_TX, can_tx_selec)) != NULL
			&& PINNODE(uint32_t)(CAN_PINCTRL_SOURCE(2, CAN_PIN_RX, can_rx_selec)) != NULL) {
			can_tx_pin_node = CAN_PINCTRL_SOURCE(2, CAN_PIN_TX, can_tx_selec);
			can_rx_pin_node = CAN_PINCTRL_SOURCE(2, CAN_PIN_RX, can_rx_selec);
			is_pin_node_initable = can_tx_pin_node->gpio_port && can_rx_pin_node->gpio_port;
		}else {
			return false;
		}
		break;
	default: return false;
	}
	
	if (is_pin_node_initable != 0) {
        lld_gpio_init(&obj->txpin, can_tx_pin_node->gpio_port, can_tx_pin_node->pin_num, GPIO_MODE_AF_PP,
						GPIO_PULLUP, GPIO_SPEED_FREQ_HIGH, can_tx_pin_node->alternate);
        lld_gpio_init(&obj->rxpin, can_rx_pin_node->gpio_port, can_rx_pin_node->pin_num, GPIO_MODE_AF_PP,
						GPIO_PULLUP, GPIO_SPEED_FREQ_HIGH, can_rx_pin_node->alternate);
	}else {
		return false;
	}
	return true;
}

void lld_can_init(lld_can_t *obj, uint8_t can, uint32_t baud, uint8_t tx_selec, uint8_t rx_selec)
{
    if (can != 1 && can != 2) return;
    obj->cannum = can;
    obj->baud = baud;
    mcu_can_list[can - 1] = obj;
    
	__HAL_RCC_FDCAN_CLK_ENABLE();
	RCC_PeriphCLKInitTypeDef FDCAN_PeriphClk = {0};
	FDCAN_PeriphClk.PeriphClockSelection = RCC_PERIPHCLK_FDCAN;
	FDCAN_PeriphClk.FdcanClockSelection = RCC_FDCANCLKSOURCE_PLL;
	HAL_RCCEx_PeriphCLKConfig(&FDCAN_PeriphClk);

	if (!lowlevel_can_pin_source_init(obj, can, tx_selec, rx_selec)) return;
	
	uint16_t presc = 10;
	if (baud == 1000) 	presc = 5;	
	if (baud == 500) 	presc = 10;	
	if (baud == 250) 	presc = 20;
	if (baud == 200) 	presc = 25;
	if (baud == 100) 	presc = 50;

	HAL_FDCAN_DeInit(&obj->hcan);
	
	if ((mcu_can_list[0] != 0) && can == 1) {
		__HAL_RCC_FDCAN_FORCE_RESET();
		__HAL_RCC_FDCAN_RELEASE_RESET();		
	}

	if (can == 1) {
		HAL_NVIC_DisableIRQ(FDCAN1_IT0_IRQn);
	}else if (can == 2) {
		HAL_NVIC_DisableIRQ(FDCAN2_IT0_IRQn);
	}
	
	if (can == 1)  obj->hcan.Instance		=	FDCAN1;
	if (can == 2)  obj->hcan.Instance		=	FDCAN2;
	/* classic mode */
	obj->hcan.Init.FrameFormat = FDCAN_FRAME_CLASSIC;
	obj->hcan.Init.Mode = FDCAN_MODE_NORMAL;
	obj->hcan.Init.AutoRetransmission = DISABLE;
	obj->hcan.Init.TransmitPause = DISABLE;
	obj->hcan.Init.ProtocolException = DISABLE;
	/* baud prescaler */
	obj->hcan.Init.NominalPrescaler = presc;
	/* can bus synchronization rejump width */
	obj->hcan.Init.NominalSyncJumpWidth = 8;
	/* tsg1 -> 2~256 */
	obj->hcan.Init.NominalTimeSeg1 = 31;
	/* tsg2 -> 2~128 */
	obj->hcan.Init.NominalTimeSeg2 = 8;
	
	if (can == 1) obj->hcan.Init.MessageRAMOffset = 0;
	if (can == 2) obj->hcan.Init.MessageRAMOffset = 1280;
	obj->hcan.Init.StdFiltersNbr = 1;
	obj->hcan.Init.ExtFiltersNbr = 0;
	
	/* �ؼ������޸� */
	obj->hcan.Init.RxFifo0ElmtsNbr = 2;
	obj->hcan.Init.RxFifo0ElmtSize = FDCAN_DATA_BYTES_8;
	obj->hcan.Init.RxFifo1ElmtsNbr = 0;
	obj->hcan.Init.RxFifo1ElmtSize = FDCAN_DATA_BYTES_8;
	obj->hcan.Init.RxBuffersNbr = 0;
	obj->hcan.Init.TxEventsNbr =	0;
	obj->hcan.Init.TxBuffersNbr = 0;
	if (can == 1) obj->hcan.Init.TxFifoQueueElmtsNbr = 2;
	if (can == 2) obj->hcan.Init.TxFifoQueueElmtsNbr = 2;
	obj->hcan.Init.TxFifoQueueMode = FDCAN_TX_FIFO_OPERATION;
	obj->hcan.Init.TxElmtSize =	FDCAN_DATA_BYTES_8;
	HAL_FDCAN_Init(&obj->hcan);
  
	FDCAN_FilterTypeDef CANx_RxFilter = {0};
	CANx_RxFilter.IdType = FDCAN_STANDARD_ID;
	if (can == 1) CANx_RxFilter.FilterIndex = 0;
	if (can == 2) CANx_RxFilter.FilterIndex = 0;
	CANx_RxFilter.FilterType = FDCAN_FILTER_MASK;
	if (can == 1) CANx_RxFilter.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;
	if (can == 2) CANx_RxFilter.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;
	CANx_RxFilter.FilterID1 = 0x0000;
	CANx_RxFilter.FilterID2 = 0x0000;
	HAL_FDCAN_ConfigFilter(&obj->hcan, &CANx_RxFilter);
	HAL_FDCAN_Start(&obj->hcan);
	
    IRQn_Type canirq[2] = {FDCAN1_IT0_IRQn, FDCAN2_IT0_IRQn};
    HAL_FDCAN_ActivateNotification(&obj->hcan, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0);
    HAL_NVIC_SetPriority(canirq[can - 1], 1, 0);
    HAL_NVIC_EnableIRQ(canirq[can - 1]);
}

void lld_can_sendframe(lld_can_t *obj, uint32_t id, uint32_t idtype, uint32_t frametype, uint8_t *data, uint8_t len)
{
	FDCAN_TxHeaderTypeDef lowlevel_msg;
	lowlevel_msg.Identifier = id;
    lowlevel_msg.IdType = idtype;
    lowlevel_msg.TxFrameType = frametype;
    lowlevel_msg.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
	lowlevel_msg.BitRateSwitch		 = FDCAN_BRS_OFF;
	lowlevel_msg.FDFormat			 = FDCAN_CLASSIC_CAN;
	lowlevel_msg.TxEventFifoControl	 = FDCAN_NO_TX_EVENTS;
	lowlevel_msg.MessageMarker		 = 0;
	switch (len) {
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
	HAL_FDCAN_AddMessageToTxFifoQ(&obj->hcan, &lowlevel_msg, &data[0]);
}

void lld_can_rx_irq(lld_can_t *obj, uint32_t fifox)
{
	FDCAN_RxHeaderTypeDef msg;
    lld_canframe_t frame;
    
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
    lld_canframelist_write(&obj->rxlist, &frame);
}

/*****************************************************************
 *****  CubeH7 CAN interrupt and callback
 ****************************************************************
*/
void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs)
{
	if ((RxFifo0ITs & FDCAN_IT_RX_FIFO0_NEW_MESSAGE) != RESET) {
		if (hfdcan->Instance == FDCAN1)
			lld_can_rx_irq(mcu_can_list[0], FDCAN_RX_FIFO0);
		else if (hfdcan->Instance == FDCAN2)
			lld_can_rx_irq(mcu_can_list[1], FDCAN_RX_FIFO0);
			
		HAL_FDCAN_ActivateNotification(hfdcan, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0);
	}
}
void HAL_FDCAN_RxFifo1Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo1ITs)
{
	if((RxFifo1ITs & FDCAN_IT_RX_FIFO1_NEW_MESSAGE) != RESET) {
		if (hfdcan->Instance == FDCAN1)
			lld_can_rx_irq(mcu_can_list[0], FDCAN_RX_FIFO1);
		else if (hfdcan->Instance == FDCAN2)
			lld_can_rx_irq(mcu_can_list[1], FDCAN_RX_FIFO1);
		
		HAL_FDCAN_ActivateNotification(hfdcan, FDCAN_IT_RX_FIFO1_NEW_MESSAGE, 0);
	}
}
/**
 * @brief CAN_IRQHandler
*/
/** CAN1 Rx FIFO0 IRQ */
void FDCAN1_IT0_IRQHandler(void)
{
	if (mcu_can_list[0]) 
		HAL_FDCAN_IRQHandler(&(mcu_can_list[0]->hcan));  
}

/** CAN1 Rx FIFO1 IRQ */
void FDCAN1_IT1_IRQHandler(void)
{
	if (mcu_can_list[0]) 
		HAL_FDCAN_IRQHandler(&(mcu_can_list[0]->hcan));   
}

/** CAN2 Rx FIFO0 IRQ */
void FDCAN2_IT0_IRQHandler(void)
{
	if (mcu_can_list[1]) 
		HAL_FDCAN_IRQHandler(&(mcu_can_list[1]->hcan));
}

/** CAN2 Rx FIFO1 IRQ */
void FDCAN2_IT1_IRQHandler(void)
{
	if (mcu_can_list[1]) 
		HAL_FDCAN_IRQHandler(&(mcu_can_list[1]->hcan)); 
}
