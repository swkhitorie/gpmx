#include "drv_can.h"

static struct can_dev_s *can_list[2] = {0, 0};

/**	
 * 	Initialize CAN Bus, include GPIO, CAN, CAN Filter, and NVIC. 
 *	the can bus baudrate computational formula is --->
 *						can bus clockFreq				200000000 (200MHz)
 *	can bus baudrate  = -------------------------- = ------------------------ = (500KHz)
 *						(ntsg1 + ntsg2 + 1) * presc 	(31 + 8 + 1) * 10
*/
#if defined (DRV_BSP_F1)/* APB1 36MHz(max) */
static const struct stm32_baud_rate_tab can_baud_rate_tab[] =
{
    {1000000, (CAN_SJW_2TQ | CAN_BS1_8TQ  | CAN_BS2_3TQ | 3)},
    {800000, (CAN_SJW_2TQ | CAN_BS1_5TQ  | CAN_BS2_3TQ | 5)},
    {500000, (CAN_SJW_2TQ | CAN_BS1_8TQ  | CAN_BS2_3TQ | 6)},
    {250000, (CAN_SJW_2TQ | CAN_BS1_8TQ  | CAN_BS2_3TQ | 12)},
    {125000, (CAN_SJW_2TQ | CAN_BS1_8TQ  | CAN_BS2_3TQ | 24)},
    {100000, (CAN_SJW_2TQ | CAN_BS1_8TQ  | CAN_BS2_3TQ | 30)},
    {50000, (CAN_SJW_2TQ | CAN_BS1_8TQ  | CAN_BS2_3TQ | 60)},
    {20000, (CAN_SJW_2TQ | CAN_BS1_8TQ  | CAN_BS2_3TQ | 150)},
    {10000, (CAN_SJW_2TQ | CAN_BS1_8TQ  | CAN_BS2_3TQ | 300)}
};
#elif defined (DRV_BSP_F4)  /* 42MHz or 45MHz */
#if defined(STM32F405xx) || defined(STM32F415xx) || defined(STM32F407xx)|| defined(STM32F417xx) ||\
    defined(STM32F401xC) || defined(STM32F401xE) /* 42MHz(max) */
static const struct stm32_baud_rate_tab can_baud_rate_tab[] =
{
    {1000000, (CAN_SJW_2TQ | CAN_BS1_9TQ  | CAN_BS2_4TQ | 3)},
    {800000, (CAN_SJW_2TQ | CAN_BS1_8TQ  | CAN_BS2_4TQ | 4)},
    {500000, (CAN_SJW_2TQ | CAN_BS1_9TQ  | CAN_BS2_4TQ | 6)},
    {250000, (CAN_SJW_2TQ | CAN_BS1_9TQ  | CAN_BS2_4TQ | 12)},
    {125000, (CAN_SJW_2TQ | CAN_BS1_9TQ  | CAN_BS2_4TQ | 24)},
    {100000, (CAN_SJW_2TQ | CAN_BS1_9TQ  | CAN_BS2_4TQ | 30)},
    {50000, (CAN_SJW_2TQ | CAN_BS1_9TQ  | CAN_BS2_4TQ | 60)},
    {20000, (CAN_SJW_2TQ | CAN_BS1_9TQ  | CAN_BS2_4TQ | 150)},
    {10000, (CAN_SJW_2TQ | CAN_BS1_9TQ  | CAN_BS2_4TQ | 300)}
};
#endif
#endif

#define BAUD_DATA(TYPE,NO)       ((can_baud_rate_tab[NO].config & TYPE##MASK))

static uint32_t get_can_baud_index(uint32_t baud)
{
    uint32_t len, idx;

    len = sizeof(can_baud_rate_tab) / sizeof(can_baud_rate_tab[0]);
    for (idx = 0; idx < len; idx++) {
        if (can_baud_rate_tab[idx].baud == baud)
            return idx;
    }

    return 0;
}

static bool _can_pinconfig(struct can_dev_s *dev);
static int  _can_config(struct can_dev_s *dev);
static int  _can_control(struct can_dev_s *dev, int cmd, void *arg);
static int  _can_sendmsg(struct can_dev_s *dev, const void *buf, uint32_t box_num);
static int  _can_recvmsg(struct can_dev_s *dev, void *buf, uint32_t fifo);
static void _can_rx_isr(struct can_dev_s *dev, uint32_t fifo);
static void _can_check_tx_complete(struct can_dev_s *dev);
static void _can_sce_isr(struct can_dev_s *dev);
static void _can_tx_isr(struct can_dev_s *dev);
static void _can_hw_isr(struct can_dev_s *dev, int event);

static int  up_can_setup(struct can_dev_s *dev);
static int  up_can_send(struct can_dev_s *dev, struct can_msg_s *msg);

const struct can_ops_s g_can_master_ops = 
{
    .co_setup = up_can_setup,
    .co_send = up_can_send,
};

bool _can_pinconfig(struct can_dev_s *dev)
{
    struct up_can_dev_s *priv = dev->cd_priv;
    uint8_t num = priv->id;
    uint8_t tx_selec = priv->pin_tx;
    uint8_t rx_selec = priv->pin_rx;

#if defined (DRV_BSP_H7)
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
        LOW_PERIPH_INITPIN(tx_pin->port, tx_pin->pin, IOMODE_AFPP, IO_PULLUP, IO_SPEEDHIGH, tx_pin->alternate);
        LOW_PERIPH_INITPIN(rx_pin->port, rx_pin->pin, IOMODE_AFPP, IO_PULLUP, IO_SPEEDHIGH, rx_pin->alternate);
	} else {
		return false;
	}
	return true;
#endif

#if defined (DRV_BSP_F1) || defined (DRV_BSP_F4)
    GPIO_TypeDef *tx_port[2]  = {GPIOA,   GPIOB  };
    uint16_t      tx_pin[2]   = {12,       6,    };

    GPIO_TypeDef *rx_port[2]  = {GPIOA,   GPIOB  };
    uint16_t      rx_pin[2]   = {11,       5,    };

    uint32_t alternate = GPIO_AF9_CAN2; // same with GPIO_AF9_CAN2

    if (tx_selec == 1 && num == 2) {
        // CAN2
        LOW_PERIPH_INITPIN(GPIOB, 13, IOMODE_AFPP, IO_NOPULL, IO_SPEEDHIGH, alternate);
        LOW_PERIPH_INITPIN(GPIOB, 12, IOMODE_AFPP, IO_NOPULL, IO_SPEEDHIGH, alternate);
    } else if (tx_selec == 1 && num == 1) {
        // CAN1
        LOW_PERIPH_INITPIN(GPIOB, 9, IOMODE_AFPP, IO_NOPULL, IO_SPEEDHIGH, alternate);
        LOW_PERIPH_INITPIN(GPIOB, 8, IOMODE_AFPP, IO_NOPULL, IO_SPEEDHIGH, alternate);
    } else if (tx_selec == 2 && num == 1) {
        // CAN1
        LOW_PERIPH_INITPIN(GPIOD, 1, IOMODE_AFPP, IO_NOPULL, IO_SPEEDHIGH, alternate);
        LOW_PERIPH_INITPIN(GPIOD, 0, IOMODE_AFPP, IO_NOPULL, IO_SPEEDHIGH, alternate);
    } else {
        LOW_PERIPH_INITPIN(tx_port[num-1], tx_pin[num-1], IOMODE_AFPP, IO_NOPULL, IO_SPEEDHIGH, alternate);
        LOW_PERIPH_INITPIN(rx_port[num-1], rx_pin[num-1], IOMODE_AFPP, IO_NOPULL, IO_SPEEDHIGH, alternate);
    }

    return true;
#endif

}

int _can_control(struct can_dev_s *dev, int cmd, void *arg)
{
    struct up_can_dev_s *priv = dev->cd_priv;
    uint8_t num = priv->id;
    uint32_t argval;

    CAN_TypeDef* can_obj[2] = {CAN1, CAN2};
    uint32_t can_rx0_irq[2] = {CAN1_RX0_IRQn, CAN2_RX0_IRQn};
    uint32_t can_rx1_irq[2] = {CAN1_RX1_IRQn, CAN2_RX1_IRQn};
    uint32_t can_tx_irq[2] = {CAN1_TX_IRQn, CAN2_TX_IRQn};
    uint32_t can_sce_irq[2] = {CAN1_SCE_IRQn, CAN2_SCE_IRQn};

    switch (cmd) {
    case CANIOC_CTRL_CLR_INT:
        argval = (uint32_t) arg;
        if (argval == CAN_DEVICE_FLAG_INT_RX) {

            HAL_NVIC_DisableIRQ(can_rx0_irq[num-1]);
            HAL_NVIC_DisableIRQ(can_rx1_irq[num-1]);
            __HAL_CAN_DISABLE_IT(&priv->hcan, CAN_IT_RX_FIFO0_MSG_PENDING);
            __HAL_CAN_DISABLE_IT(&priv->hcan, CAN_IT_RX_FIFO0_FULL);
            __HAL_CAN_DISABLE_IT(&priv->hcan, CAN_IT_RX_FIFO0_OVERRUN);
            __HAL_CAN_DISABLE_IT(&priv->hcan, CAN_IT_RX_FIFO1_MSG_PENDING);
            __HAL_CAN_DISABLE_IT(&priv->hcan, CAN_IT_RX_FIFO1_FULL);
            __HAL_CAN_DISABLE_IT(&priv->hcan, CAN_IT_RX_FIFO1_OVERRUN);
        } else if (argval == CAN_DEVICE_FLAG_INT_TX) {

            HAL_NVIC_DisableIRQ(can_tx_irq[num-1]);
            __HAL_CAN_DISABLE_IT(&priv->hcan, CAN_IT_TX_MAILBOX_EMPTY);
        } else if (argval == CAN_DEVICE_FLAG_INT_ERR) {

            HAL_NVIC_DisableIRQ(can_sce_irq[num-1]);
            __HAL_CAN_DISABLE_IT(&priv->hcan, CAN_IT_ERROR_WARNING);
            __HAL_CAN_DISABLE_IT(&priv->hcan, CAN_IT_ERROR_PASSIVE);
            __HAL_CAN_DISABLE_IT(&priv->hcan, CAN_IT_BUSOFF);
            __HAL_CAN_DISABLE_IT(&priv->hcan, CAN_IT_LAST_ERROR_CODE);
            __HAL_CAN_DISABLE_IT(&priv->hcan, CAN_IT_ERROR);
        }
        break;
    case CANIOC_CTRL_SET_INT:
        argval = (uint32_t) arg;
        if (argval == CAN_DEVICE_FLAG_INT_RX) {

            __HAL_CAN_ENABLE_IT(&priv->hcan, CAN_IT_RX_FIFO0_MSG_PENDING);
            __HAL_CAN_ENABLE_IT(&priv->hcan, CAN_IT_RX_FIFO0_FULL);
            __HAL_CAN_ENABLE_IT(&priv->hcan, CAN_IT_RX_FIFO0_OVERRUN);
            __HAL_CAN_ENABLE_IT(&priv->hcan, CAN_IT_RX_FIFO1_MSG_PENDING);
            __HAL_CAN_ENABLE_IT(&priv->hcan, CAN_IT_RX_FIFO1_FULL);
            __HAL_CAN_ENABLE_IT(&priv->hcan, CAN_IT_RX_FIFO1_OVERRUN);

            HAL_NVIC_SetPriority(can_rx0_irq[num-1], 1, 0);
            HAL_NVIC_EnableIRQ(can_rx0_irq[num-1]);
            HAL_NVIC_SetPriority(can_rx1_irq[num-1], 1, 0);
            HAL_NVIC_EnableIRQ(can_rx1_irq[num-1]);
        } else if (argval == CAN_DEVICE_FLAG_INT_TX) {

            __HAL_CAN_ENABLE_IT(&priv->hcan, CAN_IT_TX_MAILBOX_EMPTY);
            HAL_NVIC_SetPriority(can_tx_irq[num-1], 1, 0);
            HAL_NVIC_EnableIRQ(can_tx_irq[num-1]);
        } else if (argval == CAN_DEVICE_FLAG_INT_ERR) {

            __HAL_CAN_ENABLE_IT(&priv->hcan, CAN_IT_ERROR_WARNING);
            __HAL_CAN_ENABLE_IT(&priv->hcan, CAN_IT_ERROR_PASSIVE);
            __HAL_CAN_ENABLE_IT(&priv->hcan, CAN_IT_BUSOFF);
            __HAL_CAN_ENABLE_IT(&priv->hcan, CAN_IT_LAST_ERROR_CODE);
            __HAL_CAN_ENABLE_IT(&priv->hcan, CAN_IT_ERROR);

            HAL_NVIC_SetPriority(can_sce_irq[num-1], 1, 0);
            HAL_NVIC_EnableIRQ(can_sce_irq[num-1]);
        }
        break;
    case CANIOC_ADD_STDFILTER: {
        break;
    }
    case CANIOC_ADD_EXTFILTER: {
        break;
    }
    case CANIOC_SET_CONNMODES: {
        struct canioc_connmodes_s *mode = (struct canioc_connmodes_s *)arg;

        if (mode->bm_loopback != dev->cd_mode.bm_loopback ||
            mode->bm_silent != dev->cd_mode.bm_silent) {
            dev->cd_mode.bm_silent = mode->bm_silent;
            dev->cd_mode.bm_loopback = mode->bm_loopback;
            return _can_config(dev);
        }
        break;
    }
    case CANIOC_SET_BAUD:
        argval = (uint32_t) arg;
        if (argval != CAN_BAUD_1M &&
            argval != CAN_BAUD_800K &&
            argval != CAN_BAUD_500K &&
            argval != CAN_BAUD_250K &&
            argval != CAN_BAUD_125K &&
            argval != CAN_BAUD_100K &&
            argval != CAN_BAUD_50K  &&
            argval != CAN_BAUD_20K  &&
            argval != CAN_BAUD_10K) {
            argval = CAN_BAUD_1M;
        }
        if (argval != dev->cd_baud) {
            dev->cd_baud = argval;
            return _can_config(dev);
        }
        break;
    case CANIOC_GET_STATUS: {
        uint32_t errtype;
        errtype = priv->hcan.Instance->ESR;
        dev->cd_status = errtype;
        break;
    }
    case CANIOC_START:
        argval = (uint32_t)arg;
        if (argval == 0) {
            HAL_CAN_Stop(&priv->hcan);
        } else {
            HAL_CAN_Start(&priv->hcan);
        }
        break;
    }

    return 0;
}

int _can_config(struct can_dev_s *dev)
{
    struct up_can_dev_s *priv = dev->cd_priv;
    uint8_t num = priv->id;
    uint32_t baud_index;

    CAN_TypeDef* can_obj[2] = {CAN1, CAN2};

    HAL_CAN_DeInit(&priv->hcan);

    if (num == 2) {
        priv->hcan.Instance = CAN2;
        __HAL_RCC_CAN2_CLK_ENABLE();
        __HAL_RCC_CAN1_CLK_ENABLE();
    } else {
        priv->hcan.Instance = CAN1;
        __HAL_RCC_CAN1_CLK_ENABLE();
    }

    priv->hcan.Init.TimeTriggeredMode = DISABLE;
    priv->hcan.Init.AutoBusOff = ENABLE;
    priv->hcan.Init.AutoWakeUp = DISABLE;
    priv->hcan.Init.AutoRetransmission = DISABLE;
    priv->hcan.Init.ReceiveFifoLocked = DISABLE;
    priv->hcan.Init.TransmitFifoPriority = ENABLE;

    if (dev->cd_mode.bm_loopback == 0 && dev->cd_mode.bm_silent == 0) {
        priv->hcan.Init.Mode = CAN_MODE_NORMAL;
    } else if (dev->cd_mode.bm_loopback == 1 && dev->cd_mode.bm_silent == 0) {
        priv->hcan.Init.Mode = CAN_MODE_LOOPBACK;
    } else if (dev->cd_mode.bm_loopback == 0 && dev->cd_mode.bm_silent == 1) {
        priv->hcan.Init.Mode = CAN_MODE_SILENT;
    } else if (dev->cd_mode.bm_loopback == 1 && dev->cd_mode.bm_silent == 1) {
        priv->hcan.Init.Mode = CAN_MODE_SILENT_LOOPBACK;
    }

    baud_index = get_can_baud_index(dev->cd_baud);
    priv->hcan.Init.SyncJumpWidth = BAUD_DATA(SJW, baud_index);
    priv->hcan.Init.TimeSeg1 = BAUD_DATA(BS1, baud_index);
    priv->hcan.Init.TimeSeg2 = BAUD_DATA(BS2, baud_index);
    priv->hcan.Init.Prescaler = BAUD_DATA(RRESCL, baud_index);

    _can_pinconfig(dev);

    if (HAL_CAN_Init(&priv->hcan) != HAL_OK) {
        return -1;
    }

    /* default filter config */
    HAL_CAN_ConfigFilter(&priv->hcan, &priv->hfilter);

    _can_control(dev, CANIOC_CTRL_SET_INT, (void*)CAN_DEVICE_FLAG_INT_TX);
    _can_control(dev, CANIOC_CTRL_SET_INT, (void*)CAN_DEVICE_FLAG_INT_RX);
    _can_control(dev, CANIOC_CTRL_SET_INT, (void*)CAN_DEVICE_FLAG_INT_ERR);

    _can_control(dev, CANIOC_START, (void*)1);
    return 0;
}

int _can_sendmsg(struct can_dev_s *dev, const void *buf, uint32_t box_num)
{
    struct up_can_dev_s *priv = dev->cd_priv;
    uint8_t num = priv->id;
    CAN_HandleTypeDef *hcan = &priv->hcan;
    CAN_TxHeaderTypeDef txheader = {0};
    HAL_CAN_StateTypeDef state = hcan->State;
    struct can_msg_s *pmsg = (struct can_msg_s *) buf;

    if ((state == HAL_CAN_STATE_READY) ||
        (state == HAL_CAN_STATE_LISTENING)) {

        /*check select mailbox  is empty */
        switch (1 << box_num) {
        case CAN_TX_MAILBOX0:
            if (HAL_IS_BIT_SET(hcan->Instance->TSR, CAN_TSR_TME0) != SET) {
                /* Return function status */
                return -1;
            }
            break;
        case CAN_TX_MAILBOX1:
            if (HAL_IS_BIT_SET(hcan->Instance->TSR, CAN_TSR_TME1) != SET) {
                /* Return function status */
                return -1;
            }
            break;
        case CAN_TX_MAILBOX2:
            if (HAL_IS_BIT_SET(hcan->Instance->TSR, CAN_TSR_TME2) != SET) {
                /* Return function status */
                return -1;
            }
            break;
        default: break;
        }

        if (0 == pmsg->cm_hdr.ch_extid) {
            txheader.IDE = CAN_ID_STD;
            txheader.StdId = pmsg->cm_hdr.ch_id;
        } else {
            txheader.IDE = CAN_ID_EXT;
            txheader.ExtId = pmsg->cm_hdr.ch_id;
        }

        if (0 == pmsg->cm_hdr.ch_rtr) {
            txheader.RTR = CAN_RTR_DATA;
        } else {
            txheader.RTR = CAN_RTR_REMOTE;
        }

        /* clear TIR */
        hcan->Instance->sTxMailBox[box_num].TIR &= CAN_TI0R_TXRQ;

        /* Set up the Id */
        if (0 == pmsg->cm_hdr.ch_extid) {
            hcan->Instance->sTxMailBox[box_num].TIR |= (txheader.StdId << CAN_TI0R_STID_Pos) | txheader.RTR;
        } else {
            hcan->Instance->sTxMailBox[box_num].TIR |= (txheader.ExtId << CAN_TI0R_EXID_Pos) | txheader.IDE | txheader.RTR;
        }

        /* Set up the DLC */
        hcan->Instance->sTxMailBox[box_num].TDTR = pmsg->cm_hdr.ch_dlc & (uint8_t)0x0fU;

        /* Set up the data field */
        WRITE_REG(hcan->Instance->sTxMailBox[box_num].TDHR,
                ((uint32_t)pmsg->cm_data[7] << CAN_TDH0R_DATA7_Pos) |
                ((uint32_t)pmsg->cm_data[6] << CAN_TDH0R_DATA6_Pos) |
                ((uint32_t)pmsg->cm_data[5] << CAN_TDH0R_DATA5_Pos) |
                ((uint32_t)pmsg->cm_data[4] << CAN_TDH0R_DATA4_Pos));

        WRITE_REG(hcan->Instance->sTxMailBox[box_num].TDLR,
                ((uint32_t)pmsg->cm_data[3] << CAN_TDL0R_DATA3_Pos) |
                ((uint32_t)pmsg->cm_data[2] << CAN_TDL0R_DATA2_Pos) |
                ((uint32_t)pmsg->cm_data[1] << CAN_TDL0R_DATA1_Pos) |
                ((uint32_t)pmsg->cm_data[0] << CAN_TDL0R_DATA0_Pos));

        /* Request transmission */
        SET_BIT(hcan->Instance->sTxMailBox[box_num].TIR, CAN_TI0R_TXRQ);

        return 0;
    } else {
        /* Update error code */
        hcan->ErrorCode |= HAL_CAN_ERROR_NOT_INITIALIZED;

        return -1;
    }
}

int _can_recvmsg(struct can_dev_s *dev, void *buf, uint32_t fifo)
{
    struct up_can_dev_s *priv = dev->cd_priv;
    uint8_t num = priv->id;
    CAN_HandleTypeDef *hcan = &priv->hcan;
    HAL_StatusTypeDef status;
    struct can_msg_s *pmsg;
    CAN_RxHeaderTypeDef rxheader = {0};

    pmsg = (struct can_msg_s *) buf;

    /* get data */
    status = HAL_CAN_GetRxMessage(hcan, fifo, &rxheader, pmsg->cm_data);
    if (HAL_OK != status) {
        return -1;
    }

    /* get id */
    if (CAN_ID_STD == rxheader.IDE) {
        pmsg->cm_hdr.ch_extid = 0;
        pmsg->cm_hdr.ch_id = rxheader.StdId;
    } else {
        pmsg->cm_hdr.ch_extid = 1;
        pmsg->cm_hdr.ch_id = rxheader.ExtId;
    }

    /* get type */
    if (CAN_RTR_DATA == rxheader.RTR) {
        pmsg->cm_hdr.ch_rtr = 0;
    } else {
        pmsg->cm_hdr.ch_rtr = 1;
    }

    /*get rxfifo = CAN_RX_FIFO0/CAN_RX_FIFO1*/
    // pmsg->rxfifo = fifo;

    /* get len */
    pmsg->cm_hdr.ch_dlc = rxheader.DLC;

    /* get hdr_index */
    if (hcan->Instance == CAN1) {
        // pmsg->hdr_index = rxheader.FilterMatchIndex;
    } else if (hcan->Instance == CAN2) {
        // pmsg->hdr_index = rxheader.FilterMatchIndex;
    }

    return 0;
}

void _can_rx_isr(struct can_dev_s *dev, uint32_t fifo)
{
    struct up_can_dev_s *priv = dev->cd_priv;
    CAN_HandleTypeDef *hcan = &priv->hcan;

    switch (fifo) {
    case CAN_RX_FIFO0:
        /* save to user list */
        if (HAL_CAN_GetRxFifoFillLevel(hcan, CAN_RX_FIFO0) && __HAL_CAN_GET_IT_SOURCE(hcan, CAN_IT_RX_FIFO0_MSG_PENDING))
        {
            _can_hw_isr(dev, CAN_EVENT_RX_IND | fifo << 8);
        }
        /* Check FULL flag for FIFO0 */
        if (__HAL_CAN_GET_FLAG(hcan, CAN_FLAG_FF0) && __HAL_CAN_GET_IT_SOURCE(hcan, CAN_IT_RX_FIFO0_FULL))
        {
            /* Clear FIFO0 FULL Flag */
            __HAL_CAN_CLEAR_FLAG(hcan, CAN_FLAG_FF0);
        }

        /* Check Overrun flag for FIFO0 */
        if (__HAL_CAN_GET_FLAG(hcan, CAN_FLAG_FOV0) && __HAL_CAN_GET_IT_SOURCE(hcan, CAN_IT_RX_FIFO0_OVERRUN))
        {
            /* Clear FIFO0 Overrun Flag */
            __HAL_CAN_CLEAR_FLAG(hcan, CAN_FLAG_FOV0);
            _can_hw_isr(dev, CAN_EVENT_RXOF_IND | fifo << 8);
        }
        break;
    case CAN_RX_FIFO1:
        /* save to user list */
        if (HAL_CAN_GetRxFifoFillLevel(hcan, CAN_RX_FIFO1) && __HAL_CAN_GET_IT_SOURCE(hcan, CAN_IT_RX_FIFO1_MSG_PENDING))
        {
            _can_hw_isr(dev, CAN_EVENT_RX_IND | fifo << 8);
        }
        /* Check FULL flag for FIFO1 */
        if (__HAL_CAN_GET_FLAG(hcan, CAN_FLAG_FF1) && __HAL_CAN_GET_IT_SOURCE(hcan, CAN_IT_RX_FIFO1_FULL))
        {
            /* Clear FIFO1 FULL Flag */
            __HAL_CAN_CLEAR_FLAG(hcan, CAN_FLAG_FF1);
        }

        /* Check Overrun flag for FIFO1 */
        if (__HAL_CAN_GET_FLAG(hcan, CAN_FLAG_FOV1) && __HAL_CAN_GET_IT_SOURCE(hcan, CAN_IT_RX_FIFO1_OVERRUN))
        {
            /* Clear FIFO1 Overrun Flag */
            __HAL_CAN_CLEAR_FLAG(hcan, CAN_FLAG_FOV1);
            _can_hw_isr(dev, CAN_EVENT_RXOF_IND | fifo << 8);
        }
        break;
    }
}

void _can_check_tx_complete(struct can_dev_s *dev)
{
    struct up_can_dev_s *priv = dev->cd_priv;
    CAN_HandleTypeDef *hcan = &priv->hcan;

    if (__HAL_CAN_GET_FLAG(hcan, CAN_FLAG_RQCP0)) {
        if (!__HAL_CAN_GET_FLAG(hcan, CAN_FLAG_TXOK0)) {
            _can_hw_isr(dev, CAN_EVENT_TX_FAIL | 0 << 8);
        }
        SET_BIT(hcan->Instance->TSR, CAN_TSR_RQCP0);
    }

    if (__HAL_CAN_GET_FLAG(hcan, CAN_FLAG_RQCP1)) {
        if (!__HAL_CAN_GET_FLAG(hcan, CAN_FLAG_TXOK1)) {
            _can_hw_isr(dev, CAN_EVENT_TX_FAIL | 1 << 8);
        }
        SET_BIT(hcan->Instance->TSR, CAN_TSR_RQCP1);
    }

    if (__HAL_CAN_GET_FLAG(hcan, CAN_FLAG_RQCP2)) {
        if (!__HAL_CAN_GET_FLAG(hcan, CAN_FLAG_TXOK2)) {
            _can_hw_isr(dev, CAN_EVENT_TX_FAIL | 2 << 8);
        }
        SET_BIT(hcan->Instance->TSR, CAN_TSR_RQCP2);
    }

    if (__HAL_CAN_GET_FLAG(hcan, CAN_FLAG_TERR0)) {
        /*IF AutoRetransmission = ENABLE,ACK ERR handler*/
        SET_BIT(hcan->Instance->TSR, CAN_TSR_ABRQ0);/*Abort the send request, trigger the TX interrupt,release completion quantity*/
    }

    if (__HAL_CAN_GET_FLAG(hcan, CAN_FLAG_TERR1)) {
        SET_BIT(hcan->Instance->TSR, CAN_TSR_ABRQ1);
    }

    if (__HAL_CAN_GET_FLAG(hcan, CAN_FLAG_TERR2)) {
        SET_BIT(hcan->Instance->TSR, CAN_TSR_ABRQ2);
    }
}

void _can_sce_isr(struct can_dev_s *dev)
{
    struct up_can_dev_s *priv = dev->cd_priv;
    CAN_HandleTypeDef *hcan = &priv->hcan;
    uint32_t errtype = hcan->Instance->ESR;

    // printf("sce isr \r\n");
    // switch ((errtype & 0x70) >> 4) {
    //     case RT_CAN_BUS_BIT_PAD_ERR:
    //         can->status.bitpaderrcnt++;
    //         break;
    //     case RT_CAN_BUS_FORMAT_ERR:
    //         can->status.formaterrcnt++;
    //         break;
    //     case RT_CAN_BUS_ACK_ERR:/* attention !!! test ack err's unit is transmit unit */
    //         can->status.ackerrcnt++;
    //         break;
    //     case RT_CAN_BUS_IMPLICIT_BIT_ERR:
    //     case RT_CAN_BUS_EXPLICIT_BIT_ERR:
    //         can->status.biterrcnt++;
    //         break;
    //     case RT_CAN_BUS_CRC_ERR:
    //         can->status.crcerrcnt++;
    //         break;
    // }
    _can_check_tx_complete(dev);

    // can->status.lasterrtype = errtype & 0x70;
    // can->status.rcverrcnt = errtype >> 24;
    // can->status.snderrcnt = (errtype >> 16 & 0xFF);
    // can->status.errcode = errtype & 0x07;
    hcan->Instance->MSR |= CAN_MSR_ERRI;
}

void _can_tx_isr(struct can_dev_s *dev)
{
    struct up_can_dev_s *priv = dev->cd_priv;
    CAN_HandleTypeDef *hcan = &priv->hcan;

    
    if (__HAL_CAN_GET_FLAG(hcan, CAN_FLAG_RQCP0)) {
        
        if (__HAL_CAN_GET_FLAG(hcan, CAN_FLAG_TXOK0)) {

            _can_hw_isr(dev, CAN_EVENT_TX_DONE | 0 << 8);
        } else {

            _can_hw_isr(dev, CAN_EVENT_TX_FAIL | 0 << 8);
        }
        /* Write 0 to Clear transmission status flag RQCPx */
        SET_BIT(hcan->Instance->TSR, CAN_TSR_RQCP0);
    } else if (__HAL_CAN_GET_FLAG(hcan, CAN_FLAG_RQCP1)) {

        if (__HAL_CAN_GET_FLAG(hcan, CAN_FLAG_TXOK1)) {

            _can_hw_isr(dev, CAN_EVENT_TX_DONE | 1 << 8);
        } else {

            _can_hw_isr(dev, CAN_EVENT_TX_FAIL | 1 << 8);
        }
        /* Write 0 to Clear transmission status flag RQCPx */
        SET_BIT(hcan->Instance->TSR, CAN_TSR_RQCP1);
    } else if (__HAL_CAN_GET_FLAG(hcan, CAN_FLAG_RQCP2)) {

        if (__HAL_CAN_GET_FLAG(hcan, CAN_FLAG_TXOK2)) {

            _can_hw_isr(dev, CAN_EVENT_TX_DONE | 2 << 8);
        } else {

            _can_hw_isr(dev, CAN_EVENT_TX_FAIL | 2 << 8);
        }
        /* Write 0 to Clear transmission status flag RQCPx */
        SET_BIT(hcan->Instance->TSR, CAN_TSR_RQCP2);
    }
}

void _can_hw_isr(struct can_dev_s *dev, int event)
{
    struct up_can_dev_s *priv = dev->cd_priv;
    CAN_HandleTypeDef *hcan = &priv->hcan;

    switch (event & 0xff) {
    case CAN_EVENT_RXOF_IND: {

        // printf("rxof irq \r\n");
        break;
    }
    case CAN_EVENT_RX_IND: {

        struct can_msg_s tmpmsg;
        struct can_rxfifo_s *rx_fifo;
        int ch = -1;
        uint32_t no;

        rx_fifo = (struct can_rxfifo_s *)&dev->cd_rxfifo;

        no = event >> 8;
        ch = _can_recvmsg(dev, &tmpmsg, no);

        if (ch == -1) {
            break;
        }

        can_rxfifo_put(&dev->cd_rxfifo, &tmpmsg);

        break;
    }

    case CAN_EVENT_TX_DONE:
    case CAN_EVENT_TX_FAIL: {

        can_tx_post(dev);
        if ((event & 0xff) != CAN_EVENT_TX_DONE) {
            dev->cd_txfail++;
        }
        break;
    }
    }
}

int up_can_setup(struct can_dev_s *dev)
{
    int ret = 0;
    struct up_can_dev_s *priv = dev->cd_priv;
    uint8_t num = priv->id;

    priv->hfilter.FilterIdHigh = 0x0000;
    priv->hfilter.FilterIdLow = 0x0000;
    priv->hfilter.FilterMaskIdHigh = 0x0000;
    priv->hfilter.FilterMaskIdLow = 0x0000;
    priv->hfilter.FilterFIFOAssignment = CAN_FILTER_FIFO0;
    priv->hfilter.FilterMode = CAN_FILTERMODE_IDMASK;
    priv->hfilter.FilterScale = CAN_FILTERSCALE_32BIT;
    priv->hfilter.FilterActivation = ENABLE;

    priv->hfilter.FilterBank = 0;
    priv->hfilter.SlaveStartFilterBank = 14;

    if (num == 1) {
        priv->hfilter.FilterBank = 0;
    } else if (num == 2) {
        priv->hfilter.FilterBank = priv->hfilter.SlaveStartFilterBank;
    }

    _can_config(dev);

    can_list[num-1] = dev;
    return ret;
}

int up_can_send(struct can_dev_s *dev, struct can_msg_s *msg)
{
    int ret = GOK;

    if (can_tx_wait(dev) == GOK) {
        ret = _can_sendmsg(dev, msg, 0);
        if (ret == 0) {
            ret = GOK;
        } else {
            can_tx_post(dev);
            ret = -1;
        }
    } else {
        ret = -1;
    }

    return ret;
}

/****************************************************************************
 * STM32 Interrupt Callback 
 ****************************************************************************/
void CAN1_TX_IRQHandler(void)
{
    _can_tx_isr(can_list[0]);
}

void CAN1_RX0_IRQHandler(void)
{
    _can_rx_isr(can_list[0], CAN_RX_FIFO0);
}

void CAN1_RX1_IRQHandler(void)
{
    _can_rx_isr(can_list[0], CAN_RX_FIFO1);
}

void CAN1_SCE_IRQHandler(void)
{
    _can_sce_isr(can_list[0]);
}

void CAN2_TX_IRQHandler(void)
{
    _can_tx_isr(can_list[1]);
}

void CAN2_RX0_IRQHandler(void)
{
    _can_rx_isr(can_list[1], CAN_RX_FIFO0);
}

void CAN2_RX1_IRQHandler(void)
{
    _can_rx_isr(can_list[1], CAN_RX_FIFO1);
}

void CAN2_SCE_IRQHandler(void)
{
    _can_sce_isr(can_list[1]);
}
