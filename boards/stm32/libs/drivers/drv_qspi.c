#include <drv_qspi.h>

typedef QSPI_CommandTypeDef qcmd_t;

struct qspi_dev_s *qspi_dev = NULL;

static void    _qspi_pin_config(struct qspi_dev_s *dev);
static void    _qspi_dma_setup(struct qspi_dev_s *dev);
static void    _qspi_setup(struct qspi_dev_s *dev);
static qcmd_t  _qspi_setcmd(struct qspi_cmdinfo_s *cmdinfo);
static qcmd_t  _qspi_setmem(struct qspi_meminfo_s *meminfo);
static int     _qspi_command(struct qspi_dev_s *dev, struct qspi_cmdinfo_s *cmdinfo);
static int     _qspi_memory(struct qspi_dev_s *dev, struct qspi_meminfo_s *meminfo);

static int      up_qspi_setup(struct qspi_dev_s *dev);
static int      up_qspi_lock(struct qspi_dev_s *dev, bool lock);
static uint32_t up_qspi_setfrequency(struct qspi_dev_s *dev, uint32_t frequency);
static void     up_qspi_setmode(struct qspi_dev_s *dev, enum qspi_mode_e mode);
static void     up_qspi_setbits(struct qspi_dev_s *dev, int nbits);
static int      up_qspi_command(struct qspi_dev_s *dev, struct qspi_cmdinfo_s *cmdinfo);
static int      up_qspi_memory(struct qspi_dev_s *dev, struct qspi_meminfo_s *meminfo);

const struct qspi_ops_s g_qspi_ops = 
{
    .setup = up_qspi_setup,
    .lock = up_qspi_lock,
    .setfrequency = up_qspi_setfrequency,
    .setmode = up_qspi_setmode,
    .setbits = up_qspi_setbits,
    .command = up_qspi_command,
    .memory = up_qspi_memory,
};

void _qspi_pin_config(struct qspi_dev_s *dev)
{
    int ret = 0;
    struct up_qspi_dev_s *priv = dev->priv;
    struct periph_pin_t *ncspin = &priv->ncspin;
    struct periph_pin_t *sckpin = &priv->sckpin;
    struct periph_pin_t *io0pin = &priv->io0pin;
    struct periph_pin_t *io1pin = &priv->io1pin;
    struct periph_pin_t *io2pin = &priv->io2pin;
    struct periph_pin_t *io3pin = &priv->io3pin;

    LOW_PERIPH_INITPIN(ncspin->port, ncspin->pin, IOMODE_AFPP, IO_NOPULL, IO_SPEEDMAX, ncspin->alternate);
    LOW_PERIPH_INITPIN(sckpin->port, sckpin->pin, IOMODE_AFPP, IO_NOPULL, IO_SPEEDMAX, sckpin->alternate);
    LOW_PERIPH_INITPIN(io0pin->port, io0pin->pin, IOMODE_AFPP, IO_NOPULL, IO_SPEEDMAX, io0pin->alternate);
    LOW_PERIPH_INITPIN(io1pin->port, io1pin->pin, IOMODE_AFPP, IO_NOPULL, IO_SPEEDMAX, io1pin->alternate);
    LOW_PERIPH_INITPIN(io2pin->port, io2pin->pin, IOMODE_AFPP, IO_NOPULL, IO_SPEEDMAX, io2pin->alternate);
    LOW_PERIPH_INITPIN(io3pin->port, io3pin->pin, IOMODE_AFPP, IO_NOPULL, IO_SPEEDMAX, io3pin->alternate);
}

void _qspi_dma_setup(struct qspi_dev_s *dev)
{
    struct up_qspi_dev_s *priv = dev->priv;

    __HAL_RCC_MDMA_CLK_ENABLE();

    __HAL_LINKDMA(&priv->hqspi, hmdma, priv->hmdma);

    priv->hmdma.Init.TransferTriggerMode             = MDMA_BUFFER_TRANSFER;
    priv->hmdma.Init.Endianness                      = MDMA_LITTLE_ENDIANNESS_PRESERVE;
    priv->hmdma.Init.SourceInc                       = MDMA_SRC_INC_BYTE;
    priv->hmdma.Init.DestinationInc                  = MDMA_DEST_INC_DISABLE;
    priv->hmdma.Init.SourceDataSize                  = MDMA_SRC_DATASIZE_BYTE;
    priv->hmdma.Init.DestDataSize                    = MDMA_DEST_DATASIZE_BYTE;
    priv->hmdma.Init.DataAlignment                   = MDMA_DATAALIGN_PACKENABLE;
    priv->hmdma.Init.BufferTransferLength            = 128;
    priv->hmdma.Init.SourceBurst                     = MDMA_SOURCE_BURST_SINGLE;
    priv->hmdma.Init.DestBurst                       = MDMA_DEST_BURST_SINGLE;
    priv->hmdma.Init.SourceBlockAddressOffset        = 0;
    priv->hmdma.Init.DestBlockAddressOffset          = 0;
    priv->hmdma.Init.Priority                        = MDMA_PRIORITY_MEDIUM;

    priv->hmdma.Instance     = MDMA_Channel1;
    priv->hmdma.Init.Request = MDMA_REQUEST_QUADSPI_FIFO_TH;

    HAL_MDMA_Init(&priv->hmdma);

    HAL_NVIC_SetPriority(MDMA_IRQn, priv->mdma_priority, 0);
    HAL_NVIC_EnableIRQ(MDMA_IRQn);
}

void _qspi_setup(struct qspi_dev_s *dev)
{
    struct up_qspi_dev_s *priv = dev->priv;

    __HAL_RCC_QSPI_CLK_ENABLE();
    __HAL_RCC_QSPI_FORCE_RESET();
    __HAL_RCC_QSPI_RELEASE_RESET();

    HAL_QSPI_DeInit(&priv->hqspi);
    priv->hqspi.Instance = QUADSPI;
    priv->hqspi.Init.FifoThreshold = 4;
    priv->hqspi.Init.DualFlash = QSPI_DUALFLASH_DISABLE;
    priv->hqspi.Init.SampleShifting = QSPI_SAMPLE_SHIFTING_HALFCYCLE;
    priv->hqspi.Init.ChipSelectHighTime = QSPI_CS_HIGH_TIME_4_CYCLE;

    priv->hqspi.Init.FlashID = (priv->id == 1)?QSPI_FLASH_ID_1:QSPI_FLASH_ID_2;
    priv->hqspi.Init.FlashSize = POSITION_VAL(priv->medium_size) - 1;

    priv->hqspi.Init.ClockMode = QSPI_CLOCK_MODE_3;
    priv->hqspi.Init.ClockPrescaler = 2;

    HAL_QSPI_Init(&priv->hqspi);
}

qcmd_t _qspi_setcmd(struct qspi_cmdinfo_s *cmdinfo)
{
    qcmd_t stm32_quadspi_cmd;

    if (QSPICMD_ISIQUAD(cmdinfo->flags)) {
        stm32_quadspi_cmd.InstructionMode = QSPI_INSTRUCTION_4_LINES;
    } else if (QSPICMD_ISIDUAL(cmdinfo->flags)) {
        stm32_quadspi_cmd.InstructionMode = QSPI_INSTRUCTION_2_LINES;
    } else {
        stm32_quadspi_cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    }
    stm32_quadspi_cmd.Instruction = cmdinfo->cmd;

    /* XXX III option bits for 'send instruction only once' */
    stm32_quadspi_cmd.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;

    /* XXX III options for alt bytes, dummy cycles */
    stm32_quadspi_cmd.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    stm32_quadspi_cmd.DummyCycles = 0;

    if (QSPICMD_ISADDRESS(cmdinfo->flags)) {

        /* XXX III address mode mode, single, dual, quad option bits */
        stm32_quadspi_cmd.AddressMode = QSPI_ADDRESS_1_LINE;
        if (cmdinfo->addrlen == 1) {

            stm32_quadspi_cmd.AddressSize = QSPI_ADDRESS_8_BITS;
        } else if (cmdinfo->addrlen == 2) {

            stm32_quadspi_cmd.AddressSize = QSPI_ADDRESS_16_BITS;
        } else if (cmdinfo->addrlen == 3) {

            stm32_quadspi_cmd.AddressSize = QSPI_ADDRESS_24_BITS;
        } else if (cmdinfo->addrlen == 4) {

            stm32_quadspi_cmd.AddressSize = QSPI_ADDRESS_32_BITS;
        } else {
            // error
            // return -EINVAL;
        }

        stm32_quadspi_cmd.Address = cmdinfo->addr;
    } else {
        stm32_quadspi_cmd.AddressMode = QSPI_ADDRESS_NONE;
        stm32_quadspi_cmd.AddressSize = 0;
        stm32_quadspi_cmd.Address = cmdinfo->addr;
    }

    // xctn->buffer = cmdinfo->buffer;
    if (QSPICMD_ISDATA(cmdinfo->flags)) {

        /* XXX III data mode mode, single, dual, quad option bits */
        stm32_quadspi_cmd.DataMode = QSPI_DATA_1_LINE;
        stm32_quadspi_cmd.NbData = cmdinfo->buflen;

        /* XXX III double data rate option bits */
        stm32_quadspi_cmd.DdrMode = QSPI_DDR_MODE_DISABLE;
        stm32_quadspi_cmd.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
    } else {
        stm32_quadspi_cmd.DataMode = QSPI_DATA_NONE;
        stm32_quadspi_cmd.NbData = 0;
        stm32_quadspi_cmd.DdrMode = QSPI_DDR_MODE_DISABLE;
    }

    return stm32_quadspi_cmd;
}

qcmd_t _qspi_setmem(struct qspi_meminfo_s *meminfo)
{
    qcmd_t stm32_quadspi_cmd;
    /* Specify the instruction as per command info */

    /* XXX III instruction mode, single dual quad option bits */

    if (QSPIMEM_ISIQUAD(meminfo->flags)) {
        stm32_quadspi_cmd.InstructionMode = QSPI_INSTRUCTION_4_LINES;
    } else if (QSPIMEM_ISIDUAL(meminfo->flags)) {
        stm32_quadspi_cmd.InstructionMode = QSPI_INSTRUCTION_2_LINES;
    } else {
        stm32_quadspi_cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    }
    stm32_quadspi_cmd.Instruction = meminfo->cmd;

    /* XXX III option bits for 'send instruction only once' */
    stm32_quadspi_cmd.SIOOMode = QSPI_SIOO_INST_ONLY_FIRST_CMD;

    /* XXX III options for alt bytes, dummy cycles */
    stm32_quadspi_cmd.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    stm32_quadspi_cmd.AlternateBytesSize = QSPI_ALTERNATE_BYTES_8_BITS;
    stm32_quadspi_cmd.AlternateBytes = 0;
    stm32_quadspi_cmd.DummyCycles = meminfo->dummies;


    /* Specify the address size as needed */

    /* XXX III there should be a separate flags for single/dual/quad for each
     * of i,a,d
     */

    if (QSPIMEM_ISDUALIO(meminfo->flags)) {
        stm32_quadspi_cmd.AddressMode = QSPI_ADDRESS_2_LINES;
    } else if (QSPIMEM_ISQUADIO(meminfo->flags)) {
        stm32_quadspi_cmd.AddressMode = QSPI_ADDRESS_4_LINES;
    } else {
        stm32_quadspi_cmd.AddressMode = QSPI_ADDRESS_1_LINE;
    }

    if (meminfo->addrlen == 1) {
        stm32_quadspi_cmd.AddressSize = QSPI_ADDRESS_8_BITS;
    } else if (meminfo->addrlen == 2) {
        stm32_quadspi_cmd.AddressSize = QSPI_ADDRESS_16_BITS;
    } else if (meminfo->addrlen == 3) {
        stm32_quadspi_cmd.AddressSize = QSPI_ADDRESS_24_BITS;
    } else if (meminfo->addrlen == 4) {
        stm32_quadspi_cmd.AddressSize = QSPI_ADDRESS_32_BITS;
    } else {
        // error
        // return -EINVAL;
    }
    stm32_quadspi_cmd.Address = meminfo->addr;

    /* Specify the data as needed */
    // xctn->buffer = meminfo->buffer;

    /* XXX III there should be a separate flags for single/dual/quad for each
     * of i,a,d
     */

    if (QSPIMEM_ISDUALIO(meminfo->flags)) {
        stm32_quadspi_cmd.DataMode = QSPI_DATA_2_LINES;
    } else if (QSPIMEM_ISQUADIO(meminfo->flags)) {
        stm32_quadspi_cmd.DataMode = QSPI_DATA_4_LINES;
    } else {
        stm32_quadspi_cmd.DataMode = QSPI_DATA_1_LINE;
    }
    stm32_quadspi_cmd.NbData = meminfo->buflen;

    /* XXX III double data rate option bits */
    stm32_quadspi_cmd.DdrMode = QSPI_DDR_MODE_DISABLE;
    stm32_quadspi_cmd.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;

    return stm32_quadspi_cmd;
}

int up_qspi_setup(struct qspi_dev_s *dev)
{
    struct up_qspi_dev_s *priv = dev->priv;

    _qspi_pin_config(dev);

    if (priv->mdma_enable) {
        _qspi_dma_setup(dev);
    }

    _qspi_setup(dev);

    HAL_NVIC_SetPriority(QUADSPI_IRQn, priv->priority, 0);
    HAL_NVIC_EnableIRQ(QUADSPI_IRQn);

    qspi_dev = dev;
}

int _qspi_command(struct qspi_dev_s *dev, struct qspi_cmdinfo_s *cmdinfo)
{
    struct up_qspi_dev_s *priv = dev->priv;
    qcmd_t cmd = _qspi_setcmd(cmdinfo);
    int ret = 0;

    if (HAL_QSPI_Command(&priv->hqspi, &cmd, 3000) != HAL_OK)  {
        return -1;
	}

    if (QSPICMD_ISDATA(cmdinfo->flags)) {

        if (QSPICMD_ISWRITE(cmdinfo->flags)) {

            ret = HAL_QSPI_Transmit(&priv->hqspi, cmdinfo->buffer, 3000);
        } else {

            ret = HAL_QSPI_Receive(&priv->hqspi, cmdinfo->buffer, 3000);
        }
    }

    return ret;
}

int _qspi_memory(struct qspi_dev_s *dev, struct qspi_meminfo_s *meminfo)
{
    struct up_qspi_dev_s *priv = dev->priv;
    qcmd_t cmd = _qspi_setmem(meminfo);
    int ret = 0;

    if (HAL_QSPI_Command(&priv->hqspi, &cmd, 3000) != HAL_OK)  {
        return -1;
	}

    if (QSPIMEM_ISWRITE(meminfo->flags)) {

        ret = HAL_QSPI_Transmit(&priv->hqspi, meminfo->buffer, 3000);
    } else {

        if (priv->mdma_enable) {
            ret = HAL_QSPI_Receive_DMA(&priv->hqspi, meminfo->buffer);
            ret = qspi_rxwait(dev);
        } else {
            ret = HAL_QSPI_Receive(&priv->hqspi, meminfo->buffer, 3000);
        }
    }

    return ret;
}

int up_qspi_lock(struct qspi_dev_s *dev, bool lock)
{
    return qspi_devlock(dev, lock);
}

uint32_t up_qspi_setfrequency(struct qspi_dev_s *dev, uint32_t frequency)
{
    struct up_qspi_dev_s *priv = dev->priv;

    return frequency;
}

void up_qspi_setmode(struct qspi_dev_s *dev, enum qspi_mode_e mode)
{
    struct up_qspi_dev_s *priv = dev->priv;

}

void up_qspi_setbits(struct qspi_dev_s *dev, int nbits)
{
    // only support 8bits
    return;
}

int up_qspi_command(struct qspi_dev_s *dev, struct qspi_cmdinfo_s *cmdinfo)
{
    return _qspi_command(dev, cmdinfo);
}

int up_qspi_memory(struct qspi_dev_s *dev, struct qspi_meminfo_s *meminfo)
{
    return _qspi_memory(dev, meminfo);
}

/****************************************************************************
 * STM32 Callback 
 ****************************************************************************/

void HAL_QSPI_TxCpltCallback(QSPI_HandleTypeDef *hqspi)
{
    qspi_txwakeup(qspi_dev); 
}

void HAL_QSPI_RxCpltCallback(QSPI_HandleTypeDef *hqspi)
{
    qspi_rxwakeup(qspi_dev);
}

void QUADSPI_IRQHandler(void)
{
    struct up_qspi_dev_s *priv = qspi_dev->priv;
    HAL_QSPI_IRQHandler(&priv->hqspi);
}
