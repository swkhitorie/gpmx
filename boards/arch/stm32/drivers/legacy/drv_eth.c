#include "drv_eth.h"
#include <device/dnode.h>
#include <netif/ethernetif.h>
#include <lwipopts.h>

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

static ETH_HandleTypeDef EthHandle;
#if defined ( __GNUC__ ) && !defined (__CC_ARM)
static ETH_DMADescTypeDef DMARxDscrTab[ETH_RXBUFNB] __attribute__ ((aligned (4)));
static ETH_DMADescTypeDef DMATxDscrTab[ETH_TXBUFNB] __attribute__ ((aligned (4)));
static uint8_t Rx_Buff[ETH_RXBUFNB][ETH_RX_BUF_SIZE] __attribute__ ((aligned (4)));
static uint8_t Tx_Buff[ETH_TXBUFNB][ETH_TX_BUF_SIZE] __attribute__ ((aligned (4)));
#else
__align(4) ETH_DMADescTypeDef DMARxDscrTab[ETH_RXBUFNB];
__align(4) ETH_DMADescTypeDef  DMATxDscrTab[ETH_TXBUFNB];
__align(4) uint8_t Rx_Buff[ETH_RXBUFNB][ETH_RX_BUF_SIZE];
__align(4) uint8_t Tx_Buff[ETH_TXBUFNB][ETH_TX_BUF_SIZE];
#endif

static struct stm32_eth stm32_eth_device;

extern void phy_reset(void);

/* EMAC initialization function */
static int stm32_eth_init()
{
    __HAL_RCC_ETH_CLK_ENABLE();

    phy_reset();

    ETH->DMABMR |= ETH_DMABMR_SR;

    HAL_ETH_DeInit(&EthHandle);

    /* ETHERNET Configuration */
    EthHandle.Instance = ETH;
#if defined(PHY_USING_LAN8720A)
    EthHandle.Init.PhyAddress = LAN8720_PHY_ADDRESS;
#endif
    EthHandle.Init.MACAddr = (uint8_t *)&stm32_eth_device.dev_addr[0];
    EthHandle.Init.AutoNegotiation = ETH_AUTONEGOTIATION_ENABLE;
    EthHandle.Init.Speed = ETH_SPEED_100M;
    EthHandle.Init.DuplexMode = ETH_MODE_FULLDUPLEX;
    EthHandle.Init.MediaInterface = ETH_MEDIA_INTERFACE_RMII;
    EthHandle.Init.RxMode = ETH_RXINTERRUPT_MODE;
#ifdef CONFIG_LWIP_USING_HW_CHECKSUM
    EthHandle.Init.ChecksumMode = ETH_CHECKSUM_BY_HARDWARE;
#else
    EthHandle.Init.ChecksumMode = ETH_CHECKSUM_BY_SOFTWARE;
#endif

    /* configure ethernet peripheral (GPIOs, clocks, MAC, DMA) */
    if (HAL_ETH_Init(&EthHandle) != HAL_OK) {
        DLOG_E("eth hardware init failed");
    } else {
        DLOG_D("eth hardware init success");
    }

    /* Initialize Tx Descriptors list: Chain Mode */
    HAL_ETH_DMATxDescListInit(&EthHandle, DMATxDscrTab, (uint8_t *)Tx_Buff, ETH_TXBUFNB);

    /* Initialize Rx Descriptors list: Chain Mode  */
    HAL_ETH_DMARxDescListInit(&EthHandle, DMARxDscrTab, (uint8_t *)Rx_Buff, ETH_RXBUFNB);

    /* ETH interrupt Init */
    HAL_NVIC_SetPriority(ETH_IRQn, 7, 0);
    HAL_NVIC_EnableIRQ(ETH_IRQn);

    /* Enable MAC and DMA transmission and reception */
    if (HAL_ETH_Start(&EthHandle) == HAL_OK) {
        DLOG_D("emac hardware start");
    } else {
        DLOG_E("emac hardware start faild");
        return -1;
    }

    return 0;
}

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
            DLOG_D("buffer not valid");
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
    DLOG_D("transmit frame length :%d", framelength);

    /* wait for unlocked */
    while (EthHandle.Lock == HAL_LOCKED) {
        DLOG_D("wait eth lock \r\n");
    }

    state = HAL_ETH_TransmitFrame(&EthHandle, framelength);
    if (state != HAL_OK) {
        DLOG_E("eth transmit frame faild: %d", state);
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
        DLOG_D("receive frame faild");
        return NULL;
    }

    /* Obtain the size of the packet and put it into the "len" variable. */
    len = EthHandle.RxFrameInfos.length;
    buffer = (uint8_t *)EthHandle.RxFrameInfos.buffer;

    DLOG_D("receive frame len : %d", len);

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
        DLOG_E("RxCpltCallback err = %d", result);
    }
}

void HAL_ETH_ErrorCallback(ETH_HandleTypeDef *heth)
{
    DLOG_E("eth err");
}

static int stm32_eth_control(int cmd, void *args)
{
    switch (cmd) {
    case NIOCTL_GADDR:
        /* get mac address */
        if (args) {
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

/* Register the EMAC device */
int hw_stm32_eth_init(void)
{
    int state = 0;

    stm32_eth_device.eth_speed = ETH_SPEED_100M;
    stm32_eth_device.eth_mode  = ETH_MODE_FULLDUPLEX;

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
        DLOG_D("emac device init success");
    } else {
        DLOG_E("emac device init faild: %d", state);
        state = -1;
        goto __exit;
    }

__exit:
    if (state != 0) {

    }

    return state;
}

