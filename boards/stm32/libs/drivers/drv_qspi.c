#include "drv_qspi.h"
#include <string.h>

#if defined (DRV_BSP_H7)

struct up_qspi_dev_s *qspi_dev = NULL;

struct low_qspi_msg {
    const void *send_buf;
    void *recv_buf;
    size_t length;
    struct {
        uint8_t content;
        uint8_t qspi_lines;
    } instruction;
    struct {
        uint32_t content;
        uint8_t size;
        uint8_t qspi_lines;
    } address;
    struct {
        uint32_t content;
        uint8_t size;
        uint8_t qspi_lines;
    } alternate_bytes;
    uint32_t dummy_cycles;
    uint8_t qspi_data_lines;
};

typedef QSPI_CommandTypeDef qcmd_t;

static bool   low_pinconfig(struct up_qspi_dev_s *dev);
static qcmd_t low_qspi_cmd(struct low_qspi_msg *msg);

/****************************************************************************
 * Private Function
 ****************************************************************************/
bool low_pinconfig(struct up_qspi_dev_s *dev)
{
    uint8_t bank_num = dev->flashNum;
    uint8_t clks = dev->pin_clk;
    uint8_t io0s = dev->pin_io0;
    uint8_t io1s = dev->pin_io1;
    uint8_t io2s = dev->pin_io2;
    uint8_t io3s = dev->pin_io3;
    uint8_t ncs = dev->pin_ncs;
	const struct pin_node *clk_pin_node;
	const struct pin_node *io0_pin_node;
	const struct pin_node *io1_pin_node;
	const struct pin_node *io2_pin_node;
	const struct pin_node *io3_pin_node;
	const struct pin_node *ncs_pin_node;
	uint32_t illegal;

	if (QSPI_PINCTRL_SOURCE_CLK(clks) != NULL)
		clk_pin_node = QSPI_PINCTRL_SOURCE_CLK(clks);
	else
		return false;

	switch (bank_num) {
	case 1:
		if (QSPI_PINCTRL_SOURCE(1, QSPI_PIN_IO_0, io0s) != NULL &&
			QSPI_PINCTRL_SOURCE(1, QSPI_PIN_IO_1, io1s) != NULL &&
			QSPI_PINCTRL_SOURCE(1, QSPI_PIN_IO_2, io2s) != NULL &&
			QSPI_PINCTRL_SOURCE(1, QSPI_PIN_IO_3, io3s) != NULL &&
			QSPI_PINCTRL_SOURCE(1, QSPI_PIN_NCS, ncs) != NULL) {
			io0_pin_node = QSPI_PINCTRL_SOURCE(1, QSPI_PIN_IO_0, io0s);
			io1_pin_node = QSPI_PINCTRL_SOURCE(1, QSPI_PIN_IO_1, io1s);
			io2_pin_node = QSPI_PINCTRL_SOURCE(1, QSPI_PIN_IO_2, io2s);
			io3_pin_node = QSPI_PINCTRL_SOURCE(1, QSPI_PIN_IO_3, io3s);
			ncs_pin_node = QSPI_PINCTRL_SOURCE(1, QSPI_PIN_NCS, ncs);
			illegal = io0_pin_node->port && io1_pin_node->port
				&& io2_pin_node->port && io3_pin_node->port && ncs_pin_node->port;
		}else {
			return false;
		}
		break;
	case 2:
		if (QSPI_PINCTRL_SOURCE(2, QSPI_PIN_IO_0, io0s) != NULL &&
			QSPI_PINCTRL_SOURCE(2, QSPI_PIN_IO_1, io1s) != NULL &&
			QSPI_PINCTRL_SOURCE(2, QSPI_PIN_IO_2, io2s) != NULL &&
			QSPI_PINCTRL_SOURCE(2, QSPI_PIN_IO_3, io3s) != NULL &&
			QSPI_PINCTRL_SOURCE(2, QSPI_PIN_NCS, ncs) != NULL) {
			io0_pin_node = QSPI_PINCTRL_SOURCE(2, QSPI_PIN_IO_0, io0s);
			io1_pin_node = QSPI_PINCTRL_SOURCE(2, QSPI_PIN_IO_1, io1s);
			io2_pin_node = QSPI_PINCTRL_SOURCE(2, QSPI_PIN_IO_2, io2s);
			io3_pin_node = QSPI_PINCTRL_SOURCE(2, QSPI_PIN_IO_3, io3s);
			ncs_pin_node = QSPI_PINCTRL_SOURCE(2, QSPI_PIN_NCS, ncs);
			illegal = io0_pin_node->port && io1_pin_node->port
				&& io2_pin_node->port && io3_pin_node->port && ncs_pin_node->port;
		}else {
			return false;
		}			
		break;
	default: return false;
	}

	if (illegal != 0) {
        low_gpio_setup(clk_pin_node->port, clk_pin_node->pin, IOMODE_AFPP, IO_NOPULL, IO_SPEEDMAX, clk_pin_node->alternate, NULL, NULL, 0);
        low_gpio_setup(io0_pin_node->port, io0_pin_node->pin, IOMODE_AFPP, IO_NOPULL, IO_SPEEDMAX, io0_pin_node->alternate, NULL, NULL, 0);
        low_gpio_setup(io1_pin_node->port, io1_pin_node->pin, IOMODE_AFPP, IO_NOPULL, IO_SPEEDMAX, io1_pin_node->alternate, NULL, NULL, 0);
        low_gpio_setup(io2_pin_node->port, io2_pin_node->pin, IOMODE_AFPP, IO_NOPULL, IO_SPEEDMAX, io2_pin_node->alternate, NULL, NULL, 0);
        low_gpio_setup(io3_pin_node->port, io3_pin_node->pin, IOMODE_AFPP, IO_NOPULL, IO_SPEEDMAX, io3_pin_node->alternate, NULL, NULL, 0);
        low_gpio_setup(ncs_pin_node->port, ncs_pin_node->pin, IOMODE_AFPP, IO_NOPULL, IO_SPEEDMAX, ncs_pin_node->alternate, NULL, NULL, 0);
	} else {
		return false;
	}
	return true;
}

qcmd_t low_qspi_cmd(struct low_qspi_msg *msg)
{
    qcmd_t cmd;
    cmd.Instruction = msg->instruction.content;
    cmd.Address = msg->address.content;
    cmd.DummyCycles = msg->dummy_cycles;

    switch (msg->instruction.qspi_lines) {
    case 0: cmd.InstructionMode = QSPI_INSTRUCTION_NONE; break;
    case 1: cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE; break;
    case 2: cmd.InstructionMode = QSPI_INSTRUCTION_2_LINES; break;
    case 4: cmd.InstructionMode = QSPI_INSTRUCTION_4_LINES; break;
    }

    switch (msg->address.qspi_lines) {
    case 0: cmd.AddressMode = QSPI_ADDRESS_NONE; break;
    case 1: cmd.AddressMode = QSPI_ADDRESS_1_LINE; break;
    case 2: cmd.AddressMode = QSPI_ADDRESS_2_LINES; break;
    case 4: cmd.AddressMode = QSPI_ADDRESS_4_LINES; break;
    }

    if (msg->address.size == 24) {
        cmd.AddressSize = QSPI_ADDRESS_24_BITS;
    } else {
        cmd.AddressSize = QSPI_ADDRESS_32_BITS;
    }

    switch (msg->qspi_data_lines) {
    case 0: cmd.DataMode = QSPI_DATA_NONE; break;
    case 1: cmd.DataMode = QSPI_DATA_1_LINE; break;
    case 2: cmd.DataMode = QSPI_DATA_2_LINES; break;
    case 4: cmd.DataMode = QSPI_DATA_4_LINES; break;
    }

    cmd.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;
    cmd.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    cmd.DdrMode = QSPI_DDR_MODE_DISABLE;
    cmd.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
    cmd.NbData = msg->length;
    return cmd;
}

void low_qspi_init(struct up_qspi_dev_s *dev)
{
    uint8_t flashNum = dev->flashNum;
    uint8_t prescaler = dev->prescaler;
    uint32_t flashsize = dev->flashsize;

    if (flashNum != 1 && flashNum != 2) flashNum = 1;

    uint32_t flash_id[2] = { QSPI_FLASH_ID_1, QSPI_FLASH_ID_2 };

    low_pinconfig(dev);

    __HAL_RCC_QSPI_FORCE_RESET();
    __HAL_RCC_QSPI_RELEASE_RESET();	
    __HAL_RCC_QSPI_CLK_ENABLE();

    HAL_QSPI_DeInit(&dev->hqspi);
    dev->hqspi.Instance = QUADSPI;
    dev->hqspi.Init.FifoThreshold = 32;
    dev->hqspi.Init.SampleShifting = QSPI_SAMPLE_SHIFTING_HALFCYCLE;
    dev->hqspi.Init.ChipSelectHighTime = QSPI_CS_HIGH_TIME_1_CYCLE;
    dev->hqspi.Init.ClockPrescaler = (prescaler - 1);
    dev->hqspi.Init.FlashSize = flashsize;
    dev->hqspi.Init.ClockMode = QSPI_CLOCK_MODE_3;
    dev->hqspi.Init.FlashID = flash_id[flashNum - 1];
    dev->hqspi.Init.DualFlash = QSPI_DUALFLASH_DISABLE;
    HAL_QSPI_Init(&dev->hqspi);

    __HAL_RCC_MDMA_CLK_ENABLE();
    __HAL_LINKDMA(&dev->hqspi, hmdma, dev->hmdma);
    HAL_MDMA_DeInit(&dev->hmdma);
    dev->hmdma.Instance = MDMA_Channel1;
    dev->hmdma.Init.Request = MDMA_REQUEST_QUADSPI_FIFO_TH;
    dev->hmdma.Init.TransferTriggerMode = MDMA_BUFFER_TRANSFER;
    dev->hmdma.Init.Priority = MDMA_PRIORITY_HIGH;
    dev->hmdma.Init.Endianness = MDMA_LITTLE_ENDIANNESS_PRESERVE;
    dev->hmdma.Init.SourceInc = MDMA_SRC_INC_BYTE;
    dev->hmdma.Init.DestinationInc = MDMA_DEST_INC_DISABLE;
    dev->hmdma.Init.SourceDataSize = MDMA_SRC_DATASIZE_BYTE;
    dev->hmdma.Init.DestDataSize = MDMA_DEST_DATASIZE_BYTE;
    dev->hmdma.Init.DataAlignment = MDMA_DATAALIGN_PACKENABLE;
    dev->hmdma.Init.BufferTransferLength = 128;
    dev->hmdma.Init.SourceBurst = MDMA_SOURCE_BURST_SINGLE;
    dev->hmdma.Init.DestBurst = MDMA_DEST_BURST_SINGLE;
    dev->hmdma.Init.SourceBlockAddressOffset = 0;
    dev->hmdma.Init.DestBlockAddressOffset = 0;
    HAL_MDMA_Init(&dev->hmdma);
    HAL_NVIC_SetPriority(MDMA_IRQn, 0x0E, 0);
    HAL_NVIC_EnableIRQ(MDMA_IRQn);
    HAL_NVIC_SetPriority(QUADSPI_IRQn, 6, 0);
    HAL_NVIC_EnableIRQ(QUADSPI_IRQn);
    qspi_dev = dev;
}

/****************************************************************************
 * STM32 HAL Library Callback 
 ****************************************************************************/
void HAL_QSPI_RxCpltCallback(QSPI_HandleTypeDef *hqspi)
{
    qspi_dev->rx_status = 1;
}

void QUADSPI_IRQHandler(void)
{
    HAL_QSPI_IRQHandler(&qspi_dev->hqspi);
}

void MDMA_IRQHandler(void)
{
    HAL_MDMA_IRQHandler(qspi_dev->hqspi.hmdma);
}

/****************************************************************************
 * W25Qxx QSPI Flash Driver
 ****************************************************************************/
int8_t w25qxx_write_enable()
{
    struct low_qspi_msg msg;
    msg.instruction.qspi_lines = 1;
    msg.instruction.content = W25QXX_CMD_WRITE_ENABLE;
    msg.qspi_data_lines = 0;
    msg.address.qspi_lines = 0;
    msg.dummy_cycles = 0;

    qcmd_t cmd = low_qspi_cmd(&msg);
    if (HAL_QSPI_Command(&qspi_dev->hqspi, &cmd, 4000) != HAL_OK)  {
        return QSPI_W25QXX_ERROR_WRITE_ENABLE;
	}

	QSPI_AutoPollingTypeDef config = {
        .Match = 0x02,
        .MatchMode = QSPI_MATCH_MODE_AND,
        .Interval = 0x10,
        .AutomaticStop = QSPI_AUTOMATIC_STOP_ENABLE,
        .StatusBytesSize = 1,
        .Mask = W25QXX_STATUS_REG1_WEL,
    };
    msg.instruction.content = W25QXX_CMD_READSTATUS_REG1;
    msg.qspi_data_lines = 1;
    msg.length = 1;
    cmd = low_qspi_cmd(&msg);
    if (HAL_QSPI_AutoPolling(&qspi_dev->hqspi, &cmd, &config, 4000) != HAL_OK) {
        return QSPI_W25QXX_ERROR_AUTOPOLLING;
    }

    return QSPI_W25QXX_OK;
}

int8_t w25qxx_auto_poll_memready(uint32_t wait_time)
{
    struct low_qspi_msg msg;
    msg.instruction.qspi_lines = 1;
    msg.instruction.content = W25QXX_CMD_READSTATUS_REG1;
    msg.qspi_data_lines = 1;
    msg.address.qspi_lines = 0;
    msg.dummy_cycles = 0;

	qcmd_t cmd = low_qspi_cmd(&msg);
	QSPI_AutoPollingTypeDef config = {
        .Match = 0,
        .MatchMode = QSPI_MATCH_MODE_AND,
        .Interval = 0x10,
        .AutomaticStop = QSPI_AUTOMATIC_STOP_ENABLE,
        .StatusBytesSize = 1,
        .Mask = W25QXX_STATUS_REG1_BUSY,
    };
	if (HAL_QSPI_AutoPolling(&qspi_dev->hqspi, &cmd, &config, wait_time) != HAL_OK) {
		return QSPI_W25QXX_ERROR_AUTOPOLLING;
	}
	return QSPI_W25QXX_OK;
}

int8_t w25qxx_reset()
{
    struct low_qspi_msg msg;
    msg.instruction.qspi_lines = 1;
    msg.instruction.content = W25QXX_CMD_ENABLE_RESET;
    msg.qspi_data_lines = 0;
    msg.address.qspi_lines = 0;
    msg.dummy_cycles = 0;

    qcmd_t cmd = low_qspi_cmd(&msg);
	if (HAL_QSPI_Command(&qspi_dev->hqspi, &cmd, 4000) != HAL_OK) {
		return QSPI_W25QXX_ERROR_INIT;
	}

	if (w25qxx_auto_poll_memready(4000) != QSPI_W25QXX_OK) {
		return QSPI_W25QXX_ERROR_AUTOPOLLING;
	}

    msg.instruction.content = W25QXX_CMD_RESET_DEVICE; 
    cmd = low_qspi_cmd(&msg);
    if (HAL_QSPI_Command(&qspi_dev->hqspi, &cmd, 4000) != HAL_OK)  {
        return QSPI_W25QXX_ERROR_INIT;
    }
    if (w25qxx_auto_poll_memready(4000) != QSPI_W25QXX_OK) {
        return QSPI_W25QXX_ERROR_AUTOPOLLING;
    }
    return QSPI_W25QXX_OK;
}

uint32_t w25qxx_readid()
{
	uint8_t	buff[3];
	uint32_t id;

    struct low_qspi_msg msg;
    msg.instruction.qspi_lines = 1;
    msg.instruction.content = W25QXX_CMD_JEDECID;
    msg.qspi_data_lines = 1;
    msg.length = 3;
    msg.address.qspi_lines = 0;
    msg.dummy_cycles = 0;
    msg.address.size = 24;

    qcmd_t cmd = low_qspi_cmd(&msg);
    if (HAL_QSPI_Command(&qspi_dev->hqspi, &cmd, 4000) != HAL_OK)  {
        //return QSPI_W25QXX_ERROR_INIT;
    }
    if (HAL_QSPI_Receive(&qspi_dev->hqspi, buff, 4000) != HAL_OK)  {
        //return QSPI_W25QXX_ERROR_TRANSMIT;
    }

	id = (buff[0] << 16) | (buff[1] << 8 ) | buff[2];
	return id;
}

bool w25qxx_init()
{
    uint32_t device_id;
    w25qxx_reset();
    device_id = w25qxx_readid();

    return (device_id == W25QXX_FLASHID);
}

uint8_t w25qxx_memory_mapped_mode()
{
    struct low_qspi_msg msg;
    msg.instruction.qspi_lines = 1;
    msg.instruction.content = W25QXX_CMD_FASTREAD_QUADIO;
    msg.qspi_data_lines = 4;
    msg.address.qspi_lines = 4;
    msg.dummy_cycles = 6;
    msg.address.size = 24;

    qcmd_t cmd = low_qspi_cmd(&msg);
    QSPI_MemoryMappedTypeDef mem_mapped_cfg;
    mem_mapped_cfg.TimeOutActivation = QSPI_TIMEOUT_COUNTER_DISABLE;
    mem_mapped_cfg.TimeOutPeriod     = 0;

    w25qxx_reset();
    if (HAL_QSPI_MemoryMapped(&qspi_dev->hqspi, &cmd, &mem_mapped_cfg) != HAL_OK) {
        return QSPI_W25QXX_MEMORY_MAPPED;
    }
    return QSPI_W25QXX_OK;
}

uint8_t w25qxx_sector_erase(uint32_t addr)
{
    struct low_qspi_msg msg;
    msg.instruction.qspi_lines = 1;
    msg.instruction.content = W25QXX_CMD_SECTOR_ERASE;
    msg.qspi_data_lines = 0;
    msg.address.qspi_lines = 1;
    msg.dummy_cycles = 0;
    msg.address.size = 24;
    msg.address.content = addr;

    qcmd_t cmd = low_qspi_cmd(&msg);
	if (w25qxx_write_enable() != QSPI_W25QXX_OK) {
		return QSPI_W25QXX_ERROR_WRITE_ENABLE;
	}
	if (HAL_QSPI_Command(&qspi_dev->hqspi, &cmd, 4000) != HAL_OK) {
		return QSPI_W25QXX_ERROR_ERASE;
	}
    if (w25qxx_auto_poll_memready(4000) != QSPI_W25QXX_OK) {
        return QSPI_W25QXX_ERROR_AUTOPOLLING;
    }

	return QSPI_W25QXX_OK;
}

uint8_t w25qxx_blockerase_32k(uint32_t addr)
{
    struct low_qspi_msg msg;
    msg.instruction.qspi_lines = 1;
    msg.instruction.content = W25QXX_CMD_BLOCKERASE_32K;
    msg.qspi_data_lines = 0;
    msg.address.qspi_lines = 1;
    msg.dummy_cycles = 0;
    msg.address.size = 24;
    msg.address.content = addr;

    qcmd_t cmd = low_qspi_cmd(&msg);
	if (w25qxx_write_enable() != QSPI_W25QXX_OK) {
		return QSPI_W25QXX_ERROR_WRITE_ENABLE;
	}
	if (HAL_QSPI_Command(&qspi_dev->hqspi, &cmd, 4000) != HAL_OK) {
		return QSPI_W25QXX_ERROR_ERASE;
	}
    if (w25qxx_auto_poll_memready(4000) != QSPI_W25QXX_OK) {
        return QSPI_W25QXX_ERROR_AUTOPOLLING;
    }

	return QSPI_W25QXX_OK;
}

uint8_t w25qxx_blockerase_64k(uint32_t addr)
{
    struct low_qspi_msg msg;
    msg.instruction.qspi_lines = 1;
    msg.instruction.content = W25QXX_CMD_BLOCKERASE_64K;
    msg.qspi_data_lines = 0;
    msg.address.qspi_lines = 1;
    msg.dummy_cycles = 0;
    msg.address.size = 24;
    msg.address.content = addr;

    qcmd_t cmd = low_qspi_cmd(&msg);
	if (w25qxx_write_enable() != QSPI_W25QXX_OK) {
		return QSPI_W25QXX_ERROR_WRITE_ENABLE;
	}
	if (HAL_QSPI_Command(&qspi_dev->hqspi, &cmd, 4000) != HAL_OK) {
		return QSPI_W25QXX_ERROR_ERASE;
	}
    if (w25qxx_auto_poll_memready(4000) != QSPI_W25QXX_OK) {
        return QSPI_W25QXX_ERROR_AUTOPOLLING;
    }

	return QSPI_W25QXX_OK;
}

uint8_t w25qxx_chiperase()
{
    struct low_qspi_msg msg;
    msg.instruction.qspi_lines = 1;
    msg.instruction.content = W25QXX_CMD_CHIPERASE;
    msg.qspi_data_lines = 0;
    msg.address.qspi_lines = 0;
    msg.dummy_cycles = 0;

    qcmd_t cmd = low_qspi_cmd(&msg);
	if (w25qxx_write_enable() != QSPI_W25QXX_OK) {
		return QSPI_W25QXX_ERROR_WRITE_ENABLE;
	}
	if (HAL_QSPI_Command(&qspi_dev->hqspi, &cmd, 4000) != HAL_OK) {
		return QSPI_W25QXX_ERROR_ERASE;
	}
	QSPI_AutoPollingTypeDef config = {
        .Match = 0,
        .MatchMode = QSPI_MATCH_MODE_AND,
        .Interval = 0x10,
        .AutomaticStop = QSPI_AUTOMATIC_STOP_ENABLE,
        .StatusBytesSize = 1,
        .Mask = W25QXX_STATUS_REG1_BUSY,
    };
    msg.instruction.content = W25QXX_CMD_READSTATUS_REG1;
    msg.qspi_data_lines = 1;
    msg.length = 1;
    cmd = low_qspi_cmd(&msg);
	// refer option times 20s，max 100s
	if (HAL_QSPI_AutoPolling(&qspi_dev->hqspi, &cmd, &config, W25QXX_CHIPERASE_TIMEOUT_MAX) != HAL_OK) {
		return QSPI_W25QXX_ERROR_AUTOPOLLING;
	}
	return QSPI_W25QXX_OK;
}

uint8_t w25qxx_writepage(uint8_t *p, uint32_t addr, uint16_t size)
{
    struct low_qspi_msg msg;
    msg.instruction.qspi_lines = 1;
    msg.instruction.content = W25QXX_CMD_QUADINPUT_PAGEPROGRAM;
    msg.address.qspi_lines = 1;
    msg.address.size = 24;
    msg.address.content = addr;
    msg.qspi_data_lines = 4;
    msg.length = size;
    msg.dummy_cycles = 0;
    qcmd_t cmd = low_qspi_cmd(&msg);

    if (w25qxx_write_enable() != QSPI_W25QXX_OK) {
        return QSPI_W25QXX_ERROR_WRITE_ENABLE;
    }
    if (HAL_QSPI_Command(&qspi_dev->hqspi, &cmd, 4000) != HAL_OK) {
        return QSPI_W25QXX_ERROR_TRANSMIT;
    }
	if (HAL_QSPI_Transmit(&qspi_dev->hqspi, p, 4000) != HAL_OK) {
		return QSPI_W25QXX_ERROR_TRANSMIT;
	}
	if (w25qxx_auto_poll_memready(4000) != QSPI_W25QXX_OK) {
        return QSPI_W25QXX_ERROR_AUTOPOLLING;
    }
	return QSPI_W25QXX_OK;
}

uint8_t w25qxx_writebuffer(uint8_t *p, uint32_t addr, uint32_t size)
{
    uint32_t end_addr, current_size, current_addr;
    uint8_t *write_data;

    current_size = W25QXX_PAGESIZE - (addr % W25QXX_PAGESIZE);

    if (current_size > size) {
        current_size = size;
    }

	current_addr = addr;
	end_addr = addr + size;
	write_data = p;

    do {
        if (w25qxx_writepage(write_data, current_addr, current_size) != QSPI_W25QXX_OK) {
            return QSPI_W25QXX_ERROR_TRANSMIT;
        } else {
			current_addr += current_size;
			write_data += current_size;
			current_size = ((current_addr + W25QXX_PAGESIZE) > end_addr) ? 
                            (end_addr - current_addr) : W25QXX_PAGESIZE;
		}
	}
	while (current_addr < end_addr);
	return QSPI_W25QXX_OK;
}

uint8_t w25qxx_readbuffer(uint8_t *p, uint32_t addr, uint32_t size)
{
    struct low_qspi_msg msg;
    msg.instruction.qspi_lines = 1;
    msg.instruction.content = W25QXX_CMD_FASTREAD_QUADIO;
    msg.address.qspi_lines = 4;
    msg.address.size = 24;
    msg.address.content = addr;
    msg.qspi_data_lines = 4;
    msg.length = size;
    msg.dummy_cycles = 6;
    qcmd_t cmd = low_qspi_cmd(&msg);

	if (HAL_QSPI_Command(&qspi_dev->hqspi, &cmd, 4000) != HAL_OK) {
		return QSPI_W25QXX_ERROR_TRANSMIT;
	}
	if (HAL_QSPI_Receive_DMA(&qspi_dev->hqspi, p) != HAL_OK) {
		return QSPI_W25QXX_ERROR_TRANSMIT;
	}

	while(qspi_dev->rx_status == 0);
	qspi_dev->rx_status = 0;
	return QSPI_W25QXX_OK;
}

#endif
