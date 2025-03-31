#include "drv_qspi.h"
#include <string.h>

#if defined (DRV_BSP_H7)

volatile uint8_t qspi_rx_status = 0;

struct drv_qspi_t {
    QSPI_HandleTypeDef hqspi;
    MDMA_HandleTypeDef hmdma;
};

struct drv_qspi_t qspi_obj;
volatile uint8_t cmd_cplt;
volatile uint8_t rx_cplt;
volatile uint8_t tx_cplt;
volatile uint8_t status_match;

static bool drv_qspi_pin_source_init(uint8_t bank_num, uint8_t clks, 
    uint8_t io0s, uint8_t io1s, uint8_t io2s, uint8_t io3s, uint8_t ncs)
{
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
        low_gpio_setup(clk_pin_node->port, clk_pin_node->pin, IOMODE_AFPP, IO_NOPULL, IO_SPEEDMAX, clk_pin_node->alternate, NULL, 0);
        low_gpio_setup(io0_pin_node->port, io0_pin_node->pin, IOMODE_AFPP, IO_NOPULL, IO_SPEEDMAX, io0_pin_node->alternate, NULL, 0);
        low_gpio_setup(io1_pin_node->port, io1_pin_node->pin, IOMODE_AFPP, IO_NOPULL, IO_SPEEDMAX, io1_pin_node->alternate, NULL, 0);
        low_gpio_setup(io2_pin_node->port, io2_pin_node->pin, IOMODE_AFPP, IO_NOPULL, IO_SPEEDMAX, io2_pin_node->alternate, NULL, 0);
        low_gpio_setup(io3_pin_node->port, io3_pin_node->pin, IOMODE_AFPP, IO_NOPULL, IO_SPEEDMAX, io3_pin_node->alternate, NULL, 0);
        low_gpio_setup(ncs_pin_node->port, ncs_pin_node->pin, IOMODE_AFPP, IO_NOPULL, IO_SPEEDMAX, ncs_pin_node->alternate, NULL, 0);
	} else {
		return false;
	}
	return true;
}

QSPI_CommandTypeDef drv_qspi_cmd(struct drv_qspi_msg *message)
{
    QSPI_CommandTypeDef cmd;

    cmd.Instruction = message->instruction.content;
    cmd.Address = message->address.content;
    cmd.DummyCycles = message->dummy_cycles;

    switch (message->instruction.qspi_lines) {
    case 0: cmd.InstructionMode = QSPI_INSTRUCTION_NONE; break;
    case 1: cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE; break;
    case 2: cmd.InstructionMode = QSPI_INSTRUCTION_2_LINES; break;
    case 4: cmd.InstructionMode = QSPI_INSTRUCTION_4_LINES; break;
    }

    switch (message->address.qspi_lines) {
    case 0: cmd.AddressMode = QSPI_ADDRESS_NONE; break;
    case 1: cmd.AddressMode = QSPI_ADDRESS_1_LINE; break;
    case 2: cmd.AddressMode = QSPI_ADDRESS_2_LINES; break;
    case 4: cmd.AddressMode = QSPI_ADDRESS_4_LINES; break;
    }

    if (message->address.size == 24) {
        cmd.AddressSize = QSPI_ADDRESS_24_BITS;
    } else {
        cmd.AddressSize = QSPI_ADDRESS_32_BITS;
    }

    switch (message->qspi_data_lines) {
    case 0: cmd.DataMode = QSPI_DATA_NONE; break;
    case 1: cmd.DataMode = QSPI_DATA_1_LINE; break;
    case 2: cmd.DataMode = QSPI_DATA_2_LINES; break;
    case 4: cmd.DataMode = QSPI_DATA_4_LINES; break;
    }

    cmd.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;
    cmd.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    cmd.DdrMode = QSPI_DDR_MODE_DISABLE;
    cmd.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
    cmd.NbData = message->length;
    return cmd;
}

void drv_qspi_init(uint8_t flashNum, uint8_t prescaler, uint32_t flashsize,
                uint8_t clks, uint8_t io0s, uint8_t io1s, uint8_t io2s,
                uint8_t io3s, uint8_t ncss)
{
    if (flashNum != 1 && flashNum != 2) flashNum = 1;

    uint32_t flash_id[2] = { QSPI_FLASH_ID_1, QSPI_FLASH_ID_2 };

    drv_qspi_pin_source_init(flashNum, clks, io0s, io1s, io2s, io3s, ncss);

    __HAL_RCC_QSPI_FORCE_RESET();
    __HAL_RCC_QSPI_RELEASE_RESET();	
    __HAL_RCC_QSPI_CLK_ENABLE();

    HAL_QSPI_DeInit(&qspi_obj.hqspi);
    qspi_obj.hqspi.Instance = QUADSPI;
    qspi_obj.hqspi.Init.FifoThreshold = 32;
    qspi_obj.hqspi.Init.SampleShifting = QSPI_SAMPLE_SHIFTING_HALFCYCLE;
    qspi_obj.hqspi.Init.ChipSelectHighTime = QSPI_CS_HIGH_TIME_1_CYCLE;
    qspi_obj.hqspi.Init.ClockPrescaler = (prescaler - 1);
    qspi_obj.hqspi.Init.FlashSize = flashsize;
    qspi_obj.hqspi.Init.ClockMode = QSPI_CLOCK_MODE_3;
    qspi_obj.hqspi.Init.FlashID = flash_id[flashNum - 1];
    qspi_obj.hqspi.Init.DualFlash = QSPI_DUALFLASH_DISABLE;
    HAL_QSPI_Init(&qspi_obj.hqspi);

    __HAL_RCC_MDMA_CLK_ENABLE();
    __HAL_LINKDMA(&qspi_obj.hqspi, hmdma, qspi_obj.hmdma);
    HAL_MDMA_DeInit(&qspi_obj.hmdma);
    qspi_obj.hmdma.Instance = MDMA_Channel1;
    qspi_obj.hmdma.Init.Request = MDMA_REQUEST_QUADSPI_FIFO_TH;
    qspi_obj.hmdma.Init.TransferTriggerMode = MDMA_BUFFER_TRANSFER;
    qspi_obj.hmdma.Init.Priority = MDMA_PRIORITY_HIGH;
    qspi_obj.hmdma.Init.Endianness = MDMA_LITTLE_ENDIANNESS_PRESERVE;
    qspi_obj.hmdma.Init.SourceInc = MDMA_SRC_INC_BYTE;
    qspi_obj.hmdma.Init.DestinationInc = MDMA_DEST_INC_DISABLE;
    qspi_obj.hmdma.Init.SourceDataSize = MDMA_SRC_DATASIZE_BYTE;
    qspi_obj.hmdma.Init.DestDataSize = MDMA_DEST_DATASIZE_BYTE;
    qspi_obj.hmdma.Init.DataAlignment = MDMA_DATAALIGN_PACKENABLE;
    qspi_obj.hmdma.Init.BufferTransferLength = 128;
    qspi_obj.hmdma.Init.SourceBurst = MDMA_SOURCE_BURST_SINGLE;
    qspi_obj.hmdma.Init.DestBurst = MDMA_DEST_BURST_SINGLE;
    qspi_obj.hmdma.Init.SourceBlockAddressOffset = 0;
    qspi_obj.hmdma.Init.DestBlockAddressOffset = 0;
    HAL_MDMA_Init(&qspi_obj.hmdma);
    HAL_NVIC_SetPriority(MDMA_IRQn, 0x0E, 0);
    HAL_NVIC_EnableIRQ(MDMA_IRQn);

    HAL_NVIC_SetPriority(QUADSPI_IRQn, 6, 0);
    HAL_NVIC_EnableIRQ(QUADSPI_IRQn);
}

int8_t w25qxx_write_enable()
{
    struct drv_qspi_msg msg;
    msg.instruction.qspi_lines = 1;
    msg.instruction.content = W25QXX_CMD_WRITE_ENABLE;
    msg.qspi_data_lines = 0;
    msg.address.qspi_lines = 0;
    msg.dummy_cycles = 0;

    QSPI_CommandTypeDef cmd = drv_qspi_cmd(&msg);
    if (HAL_QSPI_Command(&qspi_obj.hqspi, &cmd, 4000) != HAL_OK)  {
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
    cmd = drv_qspi_cmd(&msg);
    if (HAL_QSPI_AutoPolling(&qspi_obj.hqspi, &cmd, &config, 4000) != HAL_OK) {
        return QSPI_W25QXX_ERROR_AUTOPOLLING;
    }

    return QSPI_W25QXX_OK;
}

int8_t w25qxx_auto_poll_memready(uint32_t wait_time)
{
    struct drv_qspi_msg msg;
    msg.instruction.qspi_lines = 1;
    msg.instruction.content = W25QXX_CMD_READSTATUS_REG1;
    msg.qspi_data_lines = 1;
    msg.address.qspi_lines = 0;
    msg.dummy_cycles = 0;

	QSPI_CommandTypeDef cmd = drv_qspi_cmd(&msg);
	QSPI_AutoPollingTypeDef config = {
        .Match = 0,
        .MatchMode = QSPI_MATCH_MODE_AND,
        .Interval = 0x10,
        .AutomaticStop = QSPI_AUTOMATIC_STOP_ENABLE,
        .StatusBytesSize = 1,
        .Mask = W25QXX_STATUS_REG1_BUSY,
    };
	if (HAL_QSPI_AutoPolling(&qspi_obj.hqspi, &cmd, &config, wait_time) != HAL_OK) {
		return QSPI_W25QXX_ERROR_AUTOPOLLING;
	}
	return QSPI_W25QXX_OK;
}

int8_t w25qxx_reset()
{
    struct drv_qspi_msg msg;
    msg.instruction.qspi_lines = 1;
    msg.instruction.content = W25QXX_CMD_ENABLE_RESET;
    msg.qspi_data_lines = 0;
    msg.address.qspi_lines = 0;
    msg.dummy_cycles = 0;

    QSPI_CommandTypeDef cmd = drv_qspi_cmd(&msg);
	if (HAL_QSPI_Command(&qspi_obj.hqspi, &cmd, 4000) != HAL_OK) {
		return QSPI_W25QXX_ERROR_INIT;
	}

	if (w25qxx_auto_poll_memready(4000) != QSPI_W25QXX_OK) {
		return QSPI_W25QXX_ERROR_AUTOPOLLING;
	}

    msg.instruction.content = W25QXX_CMD_RESET_DEVICE; 
    cmd = drv_qspi_cmd(&msg);
    if (HAL_QSPI_Command(&qspi_obj.hqspi, &cmd, 4000) != HAL_OK)  {
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

    struct drv_qspi_msg msg;
    msg.instruction.qspi_lines = 1;
    msg.instruction.content = W25QXX_CMD_JEDECID;
    msg.qspi_data_lines = 1;
    msg.length = 3;
    msg.address.qspi_lines = 0;
    msg.dummy_cycles = 0;
    msg.address.size = 24;

    QSPI_CommandTypeDef cmd = drv_qspi_cmd(&msg);
    if (HAL_QSPI_Command(&qspi_obj.hqspi, &cmd, 4000) != HAL_OK)  {
        //return QSPI_W25QXX_ERROR_INIT;
    }
    if (HAL_QSPI_Receive(&qspi_obj.hqspi, buff, 4000) != HAL_OK)  {
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
    struct drv_qspi_msg msg;
    msg.instruction.qspi_lines = 1;
    msg.instruction.content = W25QXX_CMD_FASTREAD_QUADIO;
    msg.qspi_data_lines = 4;
    msg.address.qspi_lines = 4;
    msg.dummy_cycles = 6;
    msg.address.size = 24;

    QSPI_CommandTypeDef cmd = drv_qspi_cmd(&msg);
    QSPI_MemoryMappedTypeDef mem_mapped_cfg;
    mem_mapped_cfg.TimeOutActivation = QSPI_TIMEOUT_COUNTER_DISABLE;
    mem_mapped_cfg.TimeOutPeriod     = 0;

    w25qxx_reset();
    if (HAL_QSPI_MemoryMapped(&qspi_obj.hqspi, &cmd, &mem_mapped_cfg) != HAL_OK) {
        return QSPI_W25QXX_MEMORY_MAPPED;
    }
    return QSPI_W25QXX_OK;
}

uint8_t w25qxx_sector_erase(uint32_t addr)
{
    struct drv_qspi_msg msg;
    msg.instruction.qspi_lines = 1;
    msg.instruction.content = W25QXX_CMD_SECTOR_ERASE;
    msg.qspi_data_lines = 0;
    msg.address.qspi_lines = 1;
    msg.dummy_cycles = 0;
    msg.address.size = 24;
    msg.address.content = addr;

    QSPI_CommandTypeDef cmd = drv_qspi_cmd(&msg);
	if (w25qxx_write_enable() != QSPI_W25QXX_OK) {
		return QSPI_W25QXX_ERROR_WRITE_ENABLE;
	}
	if (HAL_QSPI_Command(&qspi_obj.hqspi, &cmd, 4000) != HAL_OK) {
		return QSPI_W25QXX_ERROR_ERASE;
	}
    if (w25qxx_auto_poll_memready(4000) != QSPI_W25QXX_OK) {
        return QSPI_W25QXX_ERROR_AUTOPOLLING;
    }

	return QSPI_W25QXX_OK;
}

uint8_t w25qxx_blockerase_32k(uint32_t addr)
{
    struct drv_qspi_msg msg;
    msg.instruction.qspi_lines = 1;
    msg.instruction.content = W25QXX_CMD_BLOCKERASE_32K;
    msg.qspi_data_lines = 0;
    msg.address.qspi_lines = 1;
    msg.dummy_cycles = 0;
    msg.address.size = 24;
    msg.address.content = addr;

    QSPI_CommandTypeDef cmd = drv_qspi_cmd(&msg);
	if (w25qxx_write_enable() != QSPI_W25QXX_OK) {
		return QSPI_W25QXX_ERROR_WRITE_ENABLE;
	}
	if (HAL_QSPI_Command(&qspi_obj.hqspi, &cmd, 4000) != HAL_OK) {
		return QSPI_W25QXX_ERROR_ERASE;
	}
    if (w25qxx_auto_poll_memready(4000) != QSPI_W25QXX_OK) {
        return QSPI_W25QXX_ERROR_AUTOPOLLING;
    }

	return QSPI_W25QXX_OK;
}

uint8_t w25qxx_blockerase_64k(uint32_t addr)
{
    struct drv_qspi_msg msg;
    msg.instruction.qspi_lines = 1;
    msg.instruction.content = W25QXX_CMD_BLOCKERASE_64K;
    msg.qspi_data_lines = 0;
    msg.address.qspi_lines = 1;
    msg.dummy_cycles = 0;
    msg.address.size = 24;
    msg.address.content = addr;

    QSPI_CommandTypeDef cmd = drv_qspi_cmd(&msg);
	if (w25qxx_write_enable() != QSPI_W25QXX_OK) {
		return QSPI_W25QXX_ERROR_WRITE_ENABLE;
	}
	if (HAL_QSPI_Command(&qspi_obj.hqspi, &cmd, 4000) != HAL_OK) {
		return QSPI_W25QXX_ERROR_ERASE;
	}
    if (w25qxx_auto_poll_memready(4000) != QSPI_W25QXX_OK) {
        return QSPI_W25QXX_ERROR_AUTOPOLLING;
    }

	return QSPI_W25QXX_OK;
}

uint8_t w25qxx_chiperase()
{
    struct drv_qspi_msg msg;
    msg.instruction.qspi_lines = 1;
    msg.instruction.content = W25QXX_CMD_CHIPERASE;
    msg.qspi_data_lines = 0;
    msg.address.qspi_lines = 0;
    msg.dummy_cycles = 0;

    QSPI_CommandTypeDef cmd = drv_qspi_cmd(&msg);
	if (w25qxx_write_enable() != QSPI_W25QXX_OK) {
		return QSPI_W25QXX_ERROR_WRITE_ENABLE;
	}
	if (HAL_QSPI_Command(&qspi_obj.hqspi, &cmd, 4000) != HAL_OK) {
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
    cmd = drv_qspi_cmd(&msg);
	// refer option times 20s，max 100s
	if (HAL_QSPI_AutoPolling(&qspi_obj.hqspi, &cmd, &config, W25QXX_CHIPERASE_TIMEOUT_MAX) != HAL_OK) {
		return QSPI_W25QXX_ERROR_AUTOPOLLING;
	}
	return QSPI_W25QXX_OK;
}

uint8_t w25qxx_writepage(uint8_t *p, uint32_t addr, uint16_t size)
{
    struct drv_qspi_msg msg;
    msg.instruction.qspi_lines = 1;
    msg.instruction.content = W25QXX_CMD_QUADINPUT_PAGEPROGRAM;
    msg.address.qspi_lines = 1;
    msg.address.size = 24;
    msg.address.content = addr;
    msg.qspi_data_lines = 4;
    msg.length = size;
    msg.dummy_cycles = 0;
    QSPI_CommandTypeDef cmd = drv_qspi_cmd(&msg);

    if (w25qxx_write_enable() != QSPI_W25QXX_OK) {
        return QSPI_W25QXX_ERROR_WRITE_ENABLE;
    }
    if (HAL_QSPI_Command(&qspi_obj.hqspi, &cmd, 4000) != HAL_OK) {
        return QSPI_W25QXX_ERROR_TRANSMIT;
    }
	if (HAL_QSPI_Transmit(&qspi_obj.hqspi, p, 4000) != HAL_OK) {
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
    struct drv_qspi_msg msg;
    msg.instruction.qspi_lines = 1;
    msg.instruction.content = W25QXX_CMD_FASTREAD_QUADIO;
    msg.address.qspi_lines = 4;
    msg.address.size = 24;
    msg.address.content = addr;
    msg.qspi_data_lines = 4;
    msg.length = size;
    msg.dummy_cycles = 6;
    QSPI_CommandTypeDef cmd = drv_qspi_cmd(&msg);

	if (HAL_QSPI_Command(&qspi_obj.hqspi, &cmd, 4000) != HAL_OK) {
		return QSPI_W25QXX_ERROR_TRANSMIT;
	}
	if (HAL_QSPI_Receive_DMA(&qspi_obj.hqspi, p) != HAL_OK) {
		return QSPI_W25QXX_ERROR_TRANSMIT;
	}

	while(qspi_rx_status == 0);
	qspi_rx_status = 0;
	return QSPI_W25QXX_OK;
}

#ifdef QSPI_W25QXX_ENABLE_RW_TEST
static uint8_t write_buf[32 * 1024];
static uint8_t read_buf[32 * 1024];
void w25qxx_rwtest(uint32_t testaddr, char *debug)
{
    w25qxx_sector_erase(testaddr);
    for (int i = 0; i < 32 * 1024; i++) {
        write_buf[i] = i;
    }
    sprintf((char *)&write_buf[0], "W25QXX TEST String \r");
    int res = w25qxx_writebuffer(&write_buf[0], testaddr, 32 * 1024);
    res = w25qxx_readbuffer(&read_buf[0], testaddr, 32 * 1024);
    for (int i = 0; i < 128; i++) {
        if (read_buf[i] != write_buf[i]) {
            sprintf(debug, "w25qxx read check error %d\r\n", i);
            break;
        }
    }
    sprintf(debug, "[%X] w25qxx read write test success\r\n", testaddr);
}
#endif

void HAL_QSPI_RxCpltCallback(QSPI_HandleTypeDef *hqspi)
{
    qspi_rx_status = 1;
}

void QUADSPI_IRQHandler(void)
{
    HAL_QSPI_IRQHandler(&qspi_obj.hqspi);
}

void MDMA_IRQHandler(void)
{
    HAL_MDMA_IRQHandler(qspi_obj.hqspi.hmdma);
}

#endif
