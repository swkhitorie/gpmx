#include "drv_eth.h"
#include <device/dnode.h>
#include <netif/ethernetif.h>
#include <lwipopts.h>
#include <string.h>

#define LOG_TAG             "drv.emac"

#undef PHY_FULL_DUPLEX
#define PHY_LINK         (1 << 0)
#define PHY_100M         (1 << 1)
#define PHY_FULL_DUPLEX  (1 << 2)

#define MAX_ADDR_LEN     (6)

struct stm32_eth {
    struct eth_device dev;

    /* interface address info, hw address */
    uint8_t  dev_addr[MAX_ADDR_LEN];

    uint32_t eth_speed;

    /* ETH_Duplex_Mode */
    uint32_t eth_mode;
};

#if defined(DRV_STM32_H7)
static ETH_TxPacketConfig TxConfig;
#if !defined(CONFIG_ETH_PHY_ADDR_USER)
static uint32_t phy_drive_addr = 0;
#else
static uint32_t phy_drive_addr = CONFIG_ETH_PHY_ADDR_USER;
#endif
#endif

static ETH_HandleTypeDef EthHandle;
#if defined ( __GNUC__ ) && !defined (__CC_ARM)
#if !defined(DRV_STM32_H7)
static ETH_DMADescTypeDef DMARxDscrTab[ETH_RXBUFNB] __attribute__ ((aligned (4)));
static ETH_DMADescTypeDef DMATxDscrTab[ETH_TXBUFNB] __attribute__ ((aligned (4)));
static uint8_t Rx_Buff[ETH_RXBUFNB][ETH_RX_BUF_SIZE] __attribute__ ((aligned (4)));
static uint8_t Tx_Buff[ETH_TXBUFNB][ETH_TX_BUF_SIZE] __attribute__ ((aligned (4)));
#else
ETH_DMADescTypeDef DMARxDscrTab[ETH_RXBUFNB] __attribute__((section(".RxDecripSection")));
ETH_DMADescTypeDef DMATxDscrTab[ETH_TXBUFNB] __attribute__((section(".TxDecripSection")));
uint8_t Rx_Buff[ETH_RXBUFNB][ETH_RX_BUF_SIZE] __attribute__((section(".RxArraySection")));
#endif
#else
#if !defined(DRV_STM32_H7)
__align(4) ETH_DMADescTypeDef DMARxDscrTab[ETH_RXBUFNB];
__align(4) ETH_DMADescTypeDef  DMATxDscrTab[ETH_TXBUFNB];
__align(4) uint8_t Rx_Buff[ETH_RXBUFNB][ETH_RX_BUF_SIZE];
__align(4) uint8_t Tx_Buff[ETH_TXBUFNB][ETH_TX_BUF_SIZE];
#endif
#endif

static struct stm32_eth stm32_eth_device;

extern void phy_reset(void);

#if defined(DRV_STM32_H7)
static void _set_phy_drive_addr(uint16_t phy_addr)
{
#if !defined(CONFIG_ETH_PHY_ADDR_USER)
    phy_drive_addr = phy_addr;
#endif
}
#endif

static int _phy_readregister(uint16_t phy_reg, uint32_t *reg_value)
{
#if defined(DRV_STM32_H7)
    return HAL_ETH_ReadPHYRegister(&EthHandle, phy_drive_addr, phy_reg, reg_value);
#else
    return HAL_ETH_ReadPHYRegister(&EthHandle, phy_reg, reg_value);
#endif
}

static int _phy_writeregister(uint16_t phy_reg, uint32_t reg_value)
{
#if defined(DRV_STM32_H7)
    return HAL_ETH_WritePHYRegister(&EthHandle, phy_drive_addr, phy_reg, reg_value);
#else
    return HAL_ETH_WritePHYRegister(&EthHandle, phy_reg, reg_value);
#endif
}

/* EMAC initialization function */
static int stm32_eth_init()
{
#if defined(DRV_STM32_H7)
    ETH_MACConfigTypeDef mac_config;
    __HAL_RCC_ETH1MAC_CLK_ENABLE();
    __HAL_RCC_ETH1TX_CLK_ENABLE();
    __HAL_RCC_ETH1RX_CLK_ENABLE();
#else
    __HAL_RCC_ETH_CLK_ENABLE();
#endif

    phy_reset();

#if !defined(DRV_STM32_H7)
    ETH->DMABMR |= ETH_DMABMR_SR;
#else
#endif

    HAL_ETH_DeInit(&EthHandle);

    /* ETHERNET Configuration */
    EthHandle.Instance = ETH;
    EthHandle.Init.MACAddr = (uint8_t *)&stm32_eth_device.dev_addr[0];
#if defined(DRV_STM32_H7)
    EthHandle.Init.MediaInterface = HAL_ETH_RMII_MODE;
#else
    EthHandle.Init.MediaInterface = ETH_MEDIA_INTERFACE_RMII;
#endif

#if !defined(DRV_STM32_H7)

#if !defined(CONFIG_ETH_PHY_ADDR_USER)
    EthHandle.Init.AutoNegotiation = ETH_AUTONEGOTIATION_DISABLE;
#else
    EthHandle.Init.AutoNegotiation = ETH_AUTONEGOTIATION_ENABLE;
#endif
    EthHandle.Init.Speed = ETH_SPEED_100M;
    EthHandle.Init.DuplexMode = ETH_MODE_FULLDUPLEX;
    EthHandle.Init.RxMode = ETH_RXINTERRUPT_MODE;
#ifdef CONFIG_LWIP_USING_HW_CHECKSUM
    EthHandle.Init.ChecksumMode = ETH_CHECKSUM_BY_HARDWARE;
#else
    EthHandle.Init.ChecksumMode = ETH_CHECKSUM_BY_SOFTWARE;
#endif
#if defined(CONFIG_ETH_PHY_ADDR_USER)
    EthHandle.Init.PhyAddress = CONFIG_ETH_PHY_ADDR_USER;
#endif
#else  // else with !defined(DRV_STM32_H7)

    EthHandle.Init.RxDesc = DMARxDscrTab;
    EthHandle.Init.TxDesc = DMATxDscrTab;
    EthHandle.Init.RxBuffLen = ETH_RX_BUF_SIZE;
#endif // end with !defined(DRV_STM32_H7)

    /* configure ethernet peripheral (GPIOs, clocks, MAC, DMA) */
    if (HAL_ETH_Init(&EthHandle) != HAL_OK) {
        drvlog_e("[%s] hardware init failed \r\n", LOG_TAG);
    }

#if defined(DRV_STM32_H7)

    memset(&TxConfig, 0, sizeof(ETH_TxPacketConfig));
    TxConfig.Attributes   = ETH_TX_PACKETS_FEATURES_CSUM | ETH_TX_PACKETS_FEATURES_CRCPAD;
    TxConfig.ChecksumCtrl = ETH_CHECKSUM_IPHDR_PAYLOAD_INSERT_PHDR_CALC;
    TxConfig.CRCPadCtrl   = ETH_CRC_PAD_INSERT;
    for (int i = 0; i < ETH_RXBUFNB; i++) {
        HAL_ETH_DescAssignMemory(&EthHandle, i, &Rx_Buff[i][0], NULL);
    }

    HAL_ETH_SetMDIOClockRange(&EthHandle);

    HAL_ETH_GetMACConfig(&EthHandle, &mac_config);
    mac_config.DuplexMode = ETH_FULLDUPLEX_MODE;
    mac_config.Speed = ETH_SPEED_100M;
    HAL_ETH_SetMACConfig(&EthHandle, &mac_config);
#else

    /* Initialize Tx Descriptors list: Chain Mode */
    HAL_ETH_DMATxDescListInit(&EthHandle, DMATxDscrTab, (uint8_t *)Tx_Buff, ETH_TXBUFNB);

    /* Initialize Rx Descriptors list: Chain Mode  */
    HAL_ETH_DMARxDescListInit(&EthHandle, DMARxDscrTab, (uint8_t *)Rx_Buff, ETH_RXBUFNB);
#endif

    /* ETH interrupt Init */
    HAL_NVIC_SetPriority(ETH_IRQn, 7, 0);
    HAL_NVIC_EnableIRQ(ETH_IRQn);

    /* Enable MAC and DMA transmission and reception */
#if !defined(DRV_STM32_H7)
    if (HAL_ETH_Start(&EthHandle) == HAL_OK)
#else
    if (HAL_ETH_Start_IT(&EthHandle) == HAL_OK)
#endif
    {
        drvlog_d("[%s] emac hardware start\r\n", LOG_TAG);
    } else {
        drvlog_e("[%s] emac hardware start faild\r\n", LOG_TAG);
        return -1;
    }

    return 0;
}

#if !defined(DRV_STM32_H7)

/* ethernet device interface */
int stm32_eth_tx(struct pbuf *p)
{
    int ret = -1;
    HAL_StatusTypeDef state;

    struct pbuf *q;
    uint8_t *buffer = (uint8_t *)(EthHandle.TxDesc->Buffer1Addr);
    __IO ETH_DMADescTypeDef *DmaTxDesc;
    uint32_t framelength = 0;
    uint32_t bufferoffset = 0;
    uint32_t byteslefttocopy = 0;
    uint32_t payloadoffset = 0;

    DmaTxDesc = EthHandle.TxDesc;
    bufferoffset = 0;

    /* copy frame from pbufs to driver buffers */
    for (q = p; q != NULL; q = q->next) {

        /* Is this buffer available? If not, goto error */
        if ((DmaTxDesc->Status & ETH_DMATXDESC_OWN) != (uint32_t)RESET) {
            drvlog_d("[%s] buffer not valid\r\n", LOG_TAG);
            ret = -2;
            goto error;
        }

        /* Get bytes in current lwIP buffer */
        byteslefttocopy = q->len;
        payloadoffset = 0;

        /* Check if the length of data to copy is bigger than Tx buffer size*/
        while ((byteslefttocopy + bufferoffset) > ETH_TX_BUF_SIZE) {

            /* Copy data to Tx buffer*/
            SMEMCPY((uint8_t *)((uint8_t *)buffer + bufferoffset), (uint8_t *)((uint8_t *)q->payload + payloadoffset), (ETH_TX_BUF_SIZE - bufferoffset));

            /* Point to next descriptor */
            DmaTxDesc = (ETH_DMADescTypeDef *)(DmaTxDesc->Buffer2NextDescAddr);

            /* Check if the buffer is available */
            if ((DmaTxDesc->Status & ETH_DMATXDESC_OWN) != (uint32_t)RESET) {
                // LOG_E("dma tx desc buffer is not valid");
                ret = -3;
                goto error;
            }

            buffer = (uint8_t *)(DmaTxDesc->Buffer1Addr);

            byteslefttocopy = byteslefttocopy - (ETH_TX_BUF_SIZE - bufferoffset);
            payloadoffset = payloadoffset + (ETH_TX_BUF_SIZE - bufferoffset);
            framelength = framelength + (ETH_TX_BUF_SIZE - bufferoffset);
            bufferoffset = 0;
        }

        /* Copy the remaining bytes */
        SMEMCPY((uint8_t *)((uint8_t *)buffer + bufferoffset), (uint8_t *)((uint8_t *)q->payload + payloadoffset), byteslefttocopy);
        bufferoffset = bufferoffset + byteslefttocopy;
        framelength = framelength + byteslefttocopy;
    }

    /* Prepare transmit descriptors to give to DMA */
    /* TODO Optimize data send speed*/

    /* wait for unlocked */
    while (EthHandle.Lock == HAL_LOCKED) {
        drvlog_d("[%s] wait eth lock \r\n", LOG_TAG);
    }

    state = HAL_ETH_TransmitFrame(&EthHandle, framelength);
    if (state != HAL_OK) {
        drvlog_e("[%s] transmit frame faild: %d\r\n", LOG_TAG, state);
    }

    ret = ERR_OK;

error:

    /* When Transmit Underflow flag is set, clear it and issue a Transmit Poll Demand to resume transmission */
    if ((EthHandle.Instance->DMASR & ETH_DMASR_TUS) != (uint32_t)RESET) {
        /* Clear TUS ETHERNET DMA flag */
        EthHandle.Instance->DMASR = ETH_DMASR_TUS;

        /* Resume DMA transmission*/
        EthHandle.Instance->DMATPDR = 0;
    }

    return ret;
}

struct pbuf *stm32_eth_rx()
{
    struct pbuf *p = NULL;
    struct pbuf *q = NULL;
    HAL_StatusTypeDef state;

    uint16_t len = 0;
    uint8_t *buffer;
    __IO ETH_DMADescTypeDef *dmarxdesc;
    uint32_t bufferoffset = 0;
    uint32_t payloadoffset = 0;
    uint32_t byteslefttocopy = 0;
    uint32_t i = 0;

    /* Get received frame */
    state = HAL_ETH_GetReceivedFrame_IT(&EthHandle);
    if (state != HAL_OK) {
        drvlog_d("[%s] receive frame faild\r\n", LOG_TAG);
        return NULL;
    }

    /* Obtain the size of the packet and put it into the "len" variable. */
    len = EthHandle.RxFrameInfos.length;
    buffer = (uint8_t *)EthHandle.RxFrameInfos.buffer;

    // drvlog_d("[%s] receive frame len : %d\r\n", LOG_TAG, len);

    if (len > 0) {
        /* We allocate a pbuf chain of pbufs from the Lwip buffer pool */
        p = pbuf_alloc(PBUF_RAW, len, PBUF_POOL);
    }

    if (p != NULL) {
        dmarxdesc = EthHandle.RxFrameInfos.FSRxDesc;
        bufferoffset = 0;

        for (q = p; q != NULL; q = q->next) {
            byteslefttocopy = q->len;
            payloadoffset = 0;

            /* Check if the length of bytes to copy in current pbuf is bigger than Rx buffer size*/
            while ((byteslefttocopy + bufferoffset) > ETH_RX_BUF_SIZE) {
                /* Copy data to pbuf */
                SMEMCPY((uint8_t *)((uint8_t *)q->payload + payloadoffset), (uint8_t *)((uint8_t *)buffer + bufferoffset), (ETH_RX_BUF_SIZE - bufferoffset));

                /* Point to next descriptor */
                dmarxdesc = (ETH_DMADescTypeDef *)(dmarxdesc->Buffer2NextDescAddr);
                buffer = (uint8_t *)(dmarxdesc->Buffer1Addr);

                byteslefttocopy = byteslefttocopy - (ETH_RX_BUF_SIZE - bufferoffset);
                payloadoffset = payloadoffset + (ETH_RX_BUF_SIZE - bufferoffset);
                bufferoffset = 0;
            }
            /* Copy remaining data in pbuf */
            SMEMCPY((uint8_t *)((uint8_t *)q->payload + payloadoffset), (uint8_t *)((uint8_t *)buffer + bufferoffset), byteslefttocopy);
            bufferoffset = bufferoffset + byteslefttocopy;
        }
    }

    /* Release descriptors to DMA */
    /* Point to first descriptor */
    dmarxdesc = EthHandle.RxFrameInfos.FSRxDesc;
    /* Set Own bit in Rx descriptors: gives the buffers back to DMA */
    for (i = 0; i < EthHandle.RxFrameInfos.SegCount; i++) {
        dmarxdesc->Status |= ETH_DMARXDESC_OWN;
        dmarxdesc = (ETH_DMADescTypeDef *)(dmarxdesc->Buffer2NextDescAddr);
    }

    /* Clear Segment_Count */
    EthHandle.RxFrameInfos.SegCount = 0;

    /* When Rx Buffer unavailable flag is set: clear it and resume reception */
    if ((EthHandle.Instance->DMASR & ETH_DMASR_RBUS) != (uint32_t)RESET) {
        /* Clear RBUS ETHERNET DMA flag */
        EthHandle.Instance->DMASR = ETH_DMASR_RBUS;
        /* Resume DMA reception */
        EthHandle.Instance->DMARPDR = 0;
    }

    return p;
}

#else  // else with !defined(DRV_STM32_H7)

int stm32_eth_tx(struct pbuf *p)
{
    int ret = -1;
    uint32_t i = 0, framelen = 0;
    struct pbuf *q;

    HAL_StatusTypeDef state;
    ETH_BufferTypeDef Txbuffer[ETH_TX_DESC_CNT];
    memset(Txbuffer, 0, ETH_TX_DESC_CNT * sizeof(ETH_BufferTypeDef));

    /* copy frame from pbufs to driver buffers */
    for (q = p; q != NULL; q = q->next) {

        if (i >= ETH_TX_DESC_CNT) {
            return -2;
        }

        Txbuffer[i].buffer = q->payload;
        Txbuffer[i].len = q->len;

        framelen += q->len;
        if (i > 0) {
            Txbuffer[i - 1].next = &Txbuffer[i];
        }

        if (q->next == NULL) {
            Txbuffer[i].next = NULL;
        }

        i++;
    }

    TxConfig.Length = framelen;
    TxConfig.TxBuffer = Txbuffer;

    if (1) {
        SCB_CleanDCache();
        // SCB_CleanInvalidateDCache();
        state = HAL_ETH_Transmit(&EthHandle, &TxConfig, 1000);

        if (state != HAL_OK) {
            drvlog_d("[%s] eth transmit frame faild: %d\r\n", LOG_TAG, EthHandle.ErrorCode);
            EthHandle.ErrorCode = HAL_ETH_STATE_READY;
            EthHandle.gState = HAL_ETH_STATE_READY;
        }
    } else {
        drvlog_d("[%s] eth transmit frame faild, netif not up\r\n", LOG_TAG);
    }

    ret = 0;
    return 0;
}

struct pbuf *stm32_eth_rx()
{
    uint32_t framelength = 0;
    uint16_t k;
    struct pbuf *q, *p = ((void*)0);

    ETH_BufferTypeDef RxBuff;
    uint32_t alignedAddr;

    if(HAL_ETH_GetRxDataBuffer(&EthHandle, &RxBuff) == HAL_OK) {
        HAL_ETH_GetRxDataLength(&EthHandle, &framelength);
        /* Build Rx descriptor to be ready for next data reception */
        HAL_ETH_BuildRxDescriptors(&EthHandle);
        /* Invalidate data cache for ETH Rx Buffers */
        alignedAddr = (uint32_t)RxBuff.buffer & ~0x1F;
        SCB_InvalidateDCache_by_Addr((uint32_t *)alignedAddr, (uint32_t)RxBuff.buffer - alignedAddr + framelength);

        p = pbuf_alloc(PBUF_RAW, framelength, PBUF_RAM);
        if (p != NULL) {
            for (q = p, k = 0; q != NULL; q = q->next) {
                SMEMCPY((uint8_t *)q->payload, (uint8_t *)&RxBuff.buffer[k], q->len);
                k = k + q->len;
            }
        }
    }

    return p;
}
#endif  // end with !defined(DRV_STM32_H7)

/* interrupt service routine */
void ETH_IRQHandler(void)
{
    HAL_ETH_IRQHandler(&EthHandle);
}

void HAL_ETH_RxCpltCallback(ETH_HandleTypeDef *heth)
{
    int result = 0;
    result = eth_device_ready(&(stm32_eth_device.dev));
    if (result != 0) {
        drvlog_e("[%s] RxCpltCallback err = %d\r\n", LOG_TAG, result);
    }
}

void HAL_ETH_ErrorCallback(ETH_HandleTypeDef *heth)
{
    drvlog_e("[%s] err callback\r\n", LOG_TAG);
}

static int stm32_eth_control(int cmd, void *args)
{
    switch (cmd) {
    case NIOCTL_GADDR:
        /* get mac address */
        if (args) {
            // drvlog_d("dst:%08x, src:%08x\r\n", (uint32_t)args, stm32_eth_device.dev_addr);
            SMEMCPY(args, stm32_eth_device.dev_addr, 6);
        } else {
            return -1;
        }
        break;

    default :
        break;
    }

    return 0;
}

static int phy_linkchange()
{
    static uint8_t phy_speed = 0;
    uint8_t phy_speed_new = 0;
    uint32_t status;

    _phy_readregister(PHY_BASIC_STATUS_REG, (uint32_t *)&status);
    // drvlog_d("[%s] phy basic status reg is 0x%X\r\n", LOG_TAG, status);

    if (status & (PHY_AUTONEGO_COMPLETE_MASK | PHY_LINKED_STATUS_MASK)) {
        uint32_t SR = 0;

        phy_speed_new |= PHY_LINK;

        _phy_readregister(PHY_Status_REG, (uint32_t *)&SR);
        // drvlog_d("[%s] phy control status reg is 0x%X\r\n", LOG_TAG, SR);

        if (PHY_STATUS_SPEED_100M(SR)) {
            phy_speed_new |= PHY_100M;
        }

        if (PHY_STATUS_FULL_DUPLEX(SR)) {
            phy_speed_new |= PHY_FULL_DUPLEX;
        }
    }

    if (phy_speed != phy_speed_new) {
        phy_speed = phy_speed_new;
        if (phy_speed & PHY_LINK) {
            drvlog_d("[%s] link up ", LOG_TAG);
            if (phy_speed & PHY_100M) {
                drvlog_d("100Mbps ");
                stm32_eth_device.eth_speed = ETH_SPEED_100M;
            } else {
                stm32_eth_device.eth_speed = ETH_SPEED_10M;
                drvlog_d("10Mbps ");
            }

            if (phy_speed & PHY_FULL_DUPLEX) {
                drvlog_d("full-duplex \r\n");
#if !defined(DRV_STM32_H7)
                stm32_eth_device.eth_mode = ETH_MODE_FULLDUPLEX;
#else
                stm32_eth_device.eth_mode = ETH_FULLDUPLEX_MODE;
#endif
            } else {
                drvlog_d("half-duplex \r\n");
#if !defined(DRV_STM32_H7)
                stm32_eth_device.eth_mode = ETH_MODE_HALFDUPLEX;
#else
                stm32_eth_device.eth_mode = ETH_HALFDUPLEX_MODE;
#endif
            }

            /* send link up. */
            eth_device_linkchange(&stm32_eth_device.dev, 0);
            return 0;
        } else {
            drvlog_d("[%s] link down \r\n", LOG_TAG);
            eth_device_linkchange(&stm32_eth_device.dev, 1);
        }
    }

    return -1;
}

#ifdef PHY_USING_INTERRUPT_MODE
static void eth_phy_isr(void *args)
{
    uint32_t status = 0;

    _phy_readregister(PHY_INTERRUPT_FLAG_REG, (uint32_t *)&status);
    drvlog_d("phy interrupt status reg is 0x%X", status);

    phy_linkchange();
}
#endif /* PHY_USING_INTERRUPT_MODE */

static void phy_monitor_thread_entry(void *p)
{
    uint8_t phy_addr = 0xFF;
    uint8_t detected_count = 0;

    while (phy_addr == 0xFF) {
        /* phy search */
        uint32_t i, temp;
        for (i = 0; i <= 0x1F; i++) {
#if !defined(DRV_STM32_H7)
            EthHandle.Init.PhyAddress = i;
#else
            _set_phy_drive_addr(i);
#endif
            _phy_readregister(PHY_ID1_REG, (uint32_t *)&temp);
#ifdef PHY_USING_YT8512C
            if (temp != 0xFFFF)
#else
            if (temp != 0xFFFF && temp != 0x00)
#endif /* PHY_USING_YT8512C */
            {
                phy_addr = i;
                break;
            }
        }

        detected_count++;
        if (phy_addr != 0xFF) {
            break;
        }

#if defined(CONFIG_RTTNANO_ENABLE)
        rt_thread_mdelay(1000);
#elif defined(CONFIG_FREERTOS_ENABLE)
        vTaskDelay(pdMS_TO_TICKS(1000));
#endif
        if (detected_count > 10) {
            drvlog_e("No PHY device was detected, please check hardware!\r\n");
        }
    }

    /* RESET PHY */
    drvlog_d("[%s] Found a phy, address:0x%02X, RESET PHY!\r\n", LOG_TAG, phy_addr);

    if (HAL_OK != _phy_writeregister(PHY_BASIC_CONTROL_REG, PHY_RESET_MASK)) {
        drvlog_d("[%s] write BASIC_CONTROL failed!\r\n", LOG_TAG);
    }

#if 0
    uint32_t temp_sft_reset = 0xffff;
    while (temp_sft_reset & PHY_RESET_MASK) {
        _phy_readregister(PHY_BASIC_CONTROL_REG, (uint32_t *)&temp_sft_reset);
#if defined(CONFIG_RTTNANO_ENABLE)
        rt_thread_mdelay(500);
#elif defined(CONFIG_FREERTOS_ENABLE)
        vTaskDelay(pdMS_TO_TICKS(500));
#endif
    }
#else
#if defined(CONFIG_RTTNANO_ENABLE)
    rt_thread_mdelay(2000);
#elif defined(CONFIG_FREERTOS_ENABLE)
    vTaskDelay(pdMS_TO_TICKS(2000));
#endif
#endif

#if 1
    _phy_writeregister(PHY_BASIC_CONTROL_REG, PHY_AUTO_NEGOTIATION_MASK);
#else
    uint32_t regvalue = 0xffff;
    _phy_readregister(PHY_BASIC_CONTROL_REG, (uint32_t *)&regvalue);
    regvalue |= PHY_AUTO_NEGOTIATION_MASK;
    if (HAL_OK != _phy_writeregister(PHY_BASIC_CONTROL_REG, regvalue)) {
        drvlog_d("[%s] write BASIC_CONTROL failed!\r\n", LOG_TAG);
    }
#endif

#ifdef PHY_USING_INTERRUPT_MODE
    /* configuration intterrupt pin */
    stm32_gpiosetevent(GET_PINHAL(PHY_INT_PORT, PHY_INT_PIN), false, true, true, eth_phy_isr, NULL, 0);

    /* enable phy interrupt */
    _phy_writeregister(PHY_INTERRUPT_MASK_REG, PHY_INT_MASK);
#if defined(PHY_INTERRUPT_CTRL_REG)
    _phy_writeregister(PHY_INTERRUPT_CTRL_REG, PHY_INTERRUPT_EN);
#endif
#else /* PHY_USING_INTERRUPT_MODE */
    while (1) {


#if defined(CONFIG_RTTNANO_ENABLE)
        phy_linkchange();
        rt_thread_mdelay(1000);
#elif defined(CONFIG_FREERTOS_ENABLE)
        phy_linkchange();
        vTaskDelay(1000);
#else
        if (!phy_linkchange()) {
            return;
        }
        board_delay(1000);
#endif
    }
#endif
}

/* Register the EMAC device */
int hw_stm32_eth_init(void)
{
    int state = 0;

    stm32_eth_device.eth_speed = ETH_SPEED_100M;
#if !defined(DRV_STM32_H7)
    stm32_eth_device.eth_mode  = ETH_MODE_FULLDUPLEX;
#else
    stm32_eth_device.eth_mode  = ETH_FULLDUPLEX_MODE;
#endif

    /* OUI 00-80-E1 STMICROELECTRONICS. */
    stm32_eth_device.dev_addr[0] = 0x00;
    stm32_eth_device.dev_addr[1] = 0x80;
    stm32_eth_device.dev_addr[2] = 0xE1;
    /* generate MAC addr from 96bit unique ID (only for test). */
    stm32_eth_device.dev_addr[3] = *(uint8_t *)(UID_BASE + 4);
    stm32_eth_device.dev_addr[4] = *(uint8_t *)(UID_BASE + 2);
    stm32_eth_device.dev_addr[5] = *(uint8_t *)(UID_BASE + 0);

    stm32_eth_device.dev.ops.init       = stm32_eth_init;
    stm32_eth_device.dev.ops.control    = stm32_eth_control;

    stm32_eth_device.dev.eth_rx     = stm32_eth_rx;
    stm32_eth_device.dev.eth_tx     = stm32_eth_tx;

    /* register eth device */
    state = eth_device_init(&(stm32_eth_device.dev), "e0");
    if (0 == state) {
        drvlog_d("[%s] emac device init success\r\n", LOG_TAG);
    } else {
        drvlog_e("[%s] emac device init faild: %d\r\n", LOG_TAG, state);
        state = -1;
        goto __exit;
    }

#if defined(CONFIG_RTTNANO_ENABLE)
    rt_thread_t tid = rt_thread_create("phy",
                            phy_monitor_thread_entry,
                            RT_NULL,
                            1024,
                            RT_THREAD_PRIORITY_MAX - 2,
                            2);
    if (tid != RT_NULL) {
        rt_thread_startup(tid);
    } else {
        state = -1;
    }
#elif defined(CONFIG_FREERTOS_ENABLE)
    BaseType_t ret = xTaskCreate(phy_monitor_thread_entry, 
                            "phy", 
                            1024, 
                            NULL, 
                            configMAX_PRIORITIES - 2,
                            NULL);
    if (ret == pdPASS) {
        taskYIELD();
    } else {
        state = -1;
    }
#else
    phy_monitor_thread_entry();
#endif

__exit:
    if (state != 0) {

    }

    return state;
}


/*
STM32 H7 link script setting:

RxDecripSection (rw): ORIGIN = 0x30040000, LENGTH = 1K
TxDecripSection (rw): ORIGIN = 0x30040400, LENGTH = 1K
RxArraySection (rw):  ORIGIN = 0x30040800, LENGTH = 8K
.......

.RxDecripSection (NOLOAD) : ALIGN(4)
{
    . = ALIGN(4);
    *(.RxDecripSection)
    *(.RxDecripSection.*)
    . = ALIGN(4);
    RxDecripSection_free = .;
} > RxDecripSection

.TxDecripSection (NOLOAD) : ALIGN(4)
{
    . = ALIGN(4);
    *(.TxDecripSection)
    *(.TxDecripSection.*)
    . = ALIGN(4);
    TxDecripSection_free = .;
} > TxDecripSection

.RxArraySection (NOLOAD) : ALIGN(4)
{
    . = ALIGN(4);
    *(.RxArraySection)
    *(.RxArraySection.*)
    . = ALIGN(4);
    RxArraySection_free = .;
} > RxArraySection

STM32 H7 MPU setting:
    MPU_InitStruct.Enable = MPU_REGION_ENABLE;
    MPU_InitStruct.BaseAddress = 0x30040000;
    MPU_InitStruct.Size = MPU_REGION_SIZE_256B;
    MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
    MPU_InitStruct.IsBufferable = MPU_ACCESS_BUFFERABLE;
    MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
    MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
    MPU_InitStruct.Number = MPU_REGION_NUMBER5;
    MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
    MPU_InitStruct.SubRegionDisable = 0x00;
    MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;
    HAL_MPU_ConfigRegion(&MPU_InitStruct);

*/
