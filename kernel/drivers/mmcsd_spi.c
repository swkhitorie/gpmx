#include <device/mmcsd_spi.h>
#include <board_config.h>
#ifndef MMCSPI_INFO
#define MMCSPI_INFO(...) do { board_printf(__VA_ARGS__); } while(0)
#endif

typedef uint8_t command_t;
typedef uint32_t argument_t;

/* sd memory card bus commands map, refer chapter 4.7.4 */
#define CMD0        (0x40+0)     	/* GO_IDLE_STATE */
#define CMD1        (0x40+1)     	/* SEND_OP_COND */
#define CMD8        (0x40+8)     	/* SEND_IF_COND */
#define CMD9        (0x40+9)     	/* SEND_CSD */
#define CMD10       (0x40+10)    	/* SEND_CID */
#define CMD12       (0x40+12)    	/* STOP_TRANSMISSION */
#define CMD16       (0x40+16)    	/* SET_BLOCKLEN */
#define CMD17       (0x40+17)    	/* READ_SINGLE_BLOCK */
#define CMD18       (0x40+18)    	/* READ_MULTIPLE_BLOCK */
#define ACMD23      (0x40+23)    	/* SET_BLOCK_COUNT */
#define CMD24       (0x40+24)    	/* WRITE_BLOCK */
#define CMD25       (0x40+25)    	/* WRITE_MULTIPLE_BLOCK */
#define ACMD41      (0x40+41)    	/* SEND_OP_COND (ACMD) */
#define CMD55       (0x40+55)    	/* APP_CMD */
#define CMD58       (0x40+58)    	/* READ_OCR */

typedef struct __attribute__((__packed__)) {
	command_t idx;
	argument_t arg;
	uint8_t crc;
} mmcsd_cmd_t;

typedef struct {
	uint8_t bitpos;
	uint8_t bits;
} regfield;

/* CSD register structure
 * card specific data (page 225)
 */
static const regfield CSD_VER = {126, 2}; /* CSD structure version (00b) */
static const regfield TAAC = {112, 8}; /* Data Read Access Time 1 */
static const regfield NSAC = {104,8}; /* Data Read Access Time 2 in CLK (NSAC*100) */
static const regfield TRAN_SPEED = {96,8}; /* MAX Data Transfer Rate */
static const regfield CCC = {84,12}; /* Card Command Classes */
static const regfield READ_BL_LEN = {80,4}; /* MAX Read Data Block Length */
static const regfield READ_BL_PARTIAL = {79,1}; /* Partial block read allowed */
static const regfield WRITE_BLK_MISALIGN = {78,1}; /* Block Write misalignment allowed */
static const regfield READ_BLK_MISALIGN = {77,1}; /* Block Read misalignment allowed */
static const regfield DSR_IMP = {76,1}; /* Driver Stage Register is present */
static const regfield C_SIZE_v1 = {62,12}; /* Device Capacity in C_SIZE_MULT-s */
static const regfield C_SIZE_v2 = {48,22}; /* Device Capacity in blocks */
static const regfield C_SIZE_v3 = {48,22}; /* Device Capacity in blocks */
static const regfield VDD_RD_CURR_MIN = {59,3}; /* MIN Read Current at Vdd MAX */
static const regfield VDD_RD_CURR_MAX = {56,3}; /* MAX Read Current at Vdd MAX */
static const regfield VDD_WR_CURR_MIN = {53,3}; /* MIN Write Current at Vdd MAX */
static const regfield VDD_WR_CURR_MAX = {50,3}; /* MAX Write Current at Vdd MAX */
static const regfield C_SIZE_MULT = {47,3}; /* Device Capacity Multiplier */
static const regfield ERASE_BLK_EN = {46,1}; /* Erase Single Block Allowed */
static const regfield ERASE_SECTOR_SIZE = {39,7}; /* Erase Sector Size */
static const regfield WP_GRP_SIZE = {32,7}; /* Write Protect Group Size */
static const regfield WP_GRP_ENABLE = {31,1}; /* Write Protect Group Enabled */
static const regfield R2W_FACTOR = {26,3}; /* Write Speed Factor */
static const regfield WRITE_BL_LEN = {22,4}; /* MAX Write Data Block Length */
static const regfield WRITE_BL_PARTIAL = {21,1}; /* Partial Block Write Allowed */
static const regfield FILE_FMT_GRP = {15,1}; /* File Format Group (OTP) */
static const regfield COPY = {14,1}; /* Copy Flag (OTP) */
static const regfield PERM_WP = {13,1}; /* Permanent Write Protection (OTP) */
static const regfield TEMP_WP = {12,1}; /* Temporary Write Protection (R/W) */
static const regfield FILE_FMT = {10,2}; /* File Format (OTP) */
static const regfield WP_UPC = {9,1}; /* Write Protection Until Power Cycle (R/W if WP_UPC_SUPPORT) */
static const regfield CRC7 = {1,7}; /* it's obvious */
static const regfield STOP = {0,1}; /* Always 1 */

static const uint8_t dummy[16] = {
    0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 
    0xff, 0xff, 0xff, 0xff, 
    0xff, 0xff, 0xff, 0xff };

/***************************************
 *** helper functions
***************************************/
uint8_t get_crc7(const uint8_t *p, uint32_t len)
{
	const uint8_t poly = 0b10001001;
	uint8_t crc = 0;
	while (--len) {
		crc ^= *p++;
		for (uint8_t j = 0; j < 8; j++) {
			crc = (crc & 0x80u) ? ((crc << 1) ^ (poly << 1)) : (crc << 1);
		}
	}
	return crc | 1;
}

uint16_t get_crc16(const uint8_t *p, uint32_t len)
{
	uint8_t x;
	uint16_t crc = 0xFFFF;
	while (len--) {
		x = crc >> 8 ^ *p++;
		x ^= x >> 4;
		crc = (crc << 8) ^ ((uint16_t) x << 12) ^ ((uint16_t) x << 5)
				^ (uint16_t) x;
	}
	return crc;
}

uint32_t bswap32(uint32_t value)
{
    uint32_t result = 0;
    uint8_t *dst = (uint8_t *)&result;
    uint8_t *src = (uint8_t *)&value;
    dst[0] = src[3];
    dst[1] = src[2];
    dst[2] = src[1];
    dst[3] = src[0];
    return result;
}

void bswap128(void* ptr)
{
	uint32_t buf[4];
	memcpy(buf, ptr, 16);
	((uint32_t*)ptr)[0] = bswap32(buf[3]);
	((uint32_t*)ptr)[1] = bswap32(buf[2]);
	((uint32_t*)ptr)[2] = bswap32(buf[1]);
	((uint32_t*)ptr)[3] = bswap32(buf[0]);
}

uint32_t unpackreg(uint8_t *p, regfield rf)
{
	uint32_t result = 0;
	p += rf.bitpos / 8;
	rf.bitpos %= 8;
	result = (*(uint32_t*)p >> rf.bitpos) & ~(UINT32_MAX << rf.bits);
	return result;
}

/***************************************
 *** private methods
***************************************/
int mmcsd_readywait(mmcsd_obj_t *obj)
{
    return 0;
}

void mmcsd_select(mmcsd_obj_t *obj)
{
	obj->active_lock++;
    SPI_SELECT(obj->dev, SPIDEV_MMCSD(0), true);
}

void mmcsd_deselect(mmcsd_obj_t *obj)
{
	obj->active_lock--;
	if (obj->active_lock == 0)
        SPI_SELECT(obj->dev, SPIDEV_MMCSD(0), false);
}

mmcsd_bus_status_t mmcsd_recv_r1(mmcsd_obj_t *obj)
{
	uint8_t ncr = 10;
	int sta;
	do {
        sta = SPI_EXCHANGEBLOCK(obj->dev, (uint8_t *)&dummy[0], &obj->resp.r1.byte, 1);
        if (sta != SMBUS_OK)
			break;
		if (--ncr == 0) {
			sta = SMBUS_ERROR;
			break;
		}
	} while (obj->resp.r1.start);
	return sta;
}

mmcsd_bus_status_t mmcsd_recv_r2(mmcsd_obj_t *obj)
{
	int sta;
	sta = mmcsd_recv_r1(obj);
	if (sta == SMBUS_OK) {
        sta = SPI_EXCHANGEBLOCK(obj->dev, (uint8_t *)&dummy[0], &obj->resp.r2.byte, 1);
	}
	return sta;
}

mmcsd_bus_status_t mmcsd_recv_r3_r7(mmcsd_obj_t *obj)
{
	int sta;
	uint32_t buf;
	sta = mmcsd_recv_r1(obj);
	if (sta == SMBUS_OK) {
        sta = SPI_EXCHANGEBLOCK(obj->dev, (uint8_t *)&dummy[0], (uint8_t*)&buf, 4);
		if (sta == SMBUS_OK)
			obj->resp.dword = __builtin_bswap32(buf);
	}
	return sta;
}

mmcsd_bus_status_t mmcsd_cmd(mmcsd_obj_t *obj, const command_t idx, const argument_t arg)
{
	mmcsd_cmd_t cmd;
	int sta;
	if (obj->active_lock >= 2)
		return SMBUS_BUSY;

    cmd.idx = idx;
    cmd.arg = bswap32(arg);
    // cmd.crc = get_crc7((const uint8_t*)&cmd, sizeof(mmcsd_cmd_t) - 1);
	switch (idx) {
	case CMD0:
		cmd.crc = 0x95;
		break;
	case CMD8:
		cmd.crc = 0x87;
		break;
    case CMD17:
        cmd.crc = 0xff;
        break;
	case ACMD23:
	case ACMD41:
		/* All "ACMD" command is a sequence of CMD55, CMD<n> */
		sta = mmcsd_cmd(obj, CMD55, 0);
		if (sta != SMBUS_OK)
			return sta;
		//break is omitted intentionally
	default:
		/* All other commands skip CRC check (by default but can be activated) */
		cmd.crc = 0x01;
	}

    mmcsd_select(obj);
    sta = SPI_SNDBLOCK(obj->dev, (uint8_t*)&cmd, sizeof(mmcsd_cmd_t));
	if (sta == SMBUS_OK) {
		switch (idx) {
		case CMD8:
			sta = mmcsd_recv_r3_r7(obj);
			obj->resp_type = RT_R7;
			break;
		case CMD58:
			sta = mmcsd_recv_r3_r7(obj);
			obj->resp_type = RT_R3;
			break;
		default:
			sta = mmcsd_recv_r1(obj);
			obj->resp_type = RT_R1;
		}
		if (sta == SMBUS_OK) {
			if (obj->resp.r1.byte & ~R1_IDLE)
				sta = SMBUS_ERROR;
		}
	}
    mmcsd_deselect(obj);
	return sta;
}

mmcsd_bus_status_t mmcsd_read_datablock(mmcsd_obj_t *obj, uint8_t *p, uint16_t size)
{
	uint16_t retry_cnt = 0;
	uint16_t crc16;
	int sta;
	uint8_t token;

	/* Pooling for a valid Data Token */
	do {
		retry_cnt++;
        sta = SPI_EXCHANGEBLOCK(obj->dev, (uint8_t *)&dummy[0], &token, 1);
	} while (sta == SMBUS_OK && token == 0xff);

	if (sta != SMBUS_OK) {
		obj->errorToken = token;
		return sta;
	}
	if (token != 0xfe) {
		obj->errorToken = token;
		return SMBUS_ERROR;
	}
	/* Receiving data block (16 bytes at a time) */
	while (size) {
		uint16_t readSize;
		if (size > 16) {
			readSize = 16;
			size -= 16;
		} else {
			readSize = size;
			size = 0;
		}
        sta = SPI_EXCHANGEBLOCK(obj->dev, (uint8_t *)&dummy[0], p, readSize);
		if (sta != SMBUS_OK)
			return sta;
		p += 16;
	}
	/* Receive CRC but discarding it for now */
    sta = SPI_EXCHANGEBLOCK(obj->dev, (uint8_t *)&dummy[0], (uint8_t*)&crc16, 2);
	return sta;
}

mmcsd_state_t mmcsd_cmd0_goidlestate(mmcsd_obj_t *obj)
{
    mmcsd_bus_status_t sta;
    sta = SPI_SNDBLOCK(obj->dev, (uint8_t *)&dummy[0], 10);
	if (sta != SMBUS_OK) {
		obj->state = SM_STATE_RESET;
		return obj->state;
	}

    /* CMD0: Reset all card into idle state */
	sta = mmcsd_cmd(obj, CMD0, 0);
	if (sta != SMBUS_OK || obj->resp.r1.byte != R1_IDLE) {
		obj->state = SM_STATE_ERROR;
		return obj->state;
	}
    return SM_STATE_READY;
}

mmcsd_state_t mmcsd_cmd8_optcondvalidation(mmcsd_obj_t *obj)
{
    mmcsd_bus_status_t sta;
    uint8_t retry;
    /* CMD8: send sd card condition and support info */
    /* support voltage 2.7~3.6v + check mode 10101010b */
	sta = mmcsd_cmd(obj, CMD8, 0x1AA);
	if (sta == SMBUS_OK) {
        /* CMD8 response, SD V2.0, capacity > 2G */
		retry = obj->max_retry;
		do {
            /* ACMD41 Send, HCS=1, send CMD55(APP_CMD) first */
			sta = mmcsd_cmd(obj, ACMD41, 0x40000000);
			if (sta != SMBUS_OK || --retry == 0) {
				obj->state = SM_STATE_ERROR;
				return obj->state; // return state
			}
		} while (obj->resp.r1.byte != 0);
        
        /* CMD58 Read OCR */
		sta = mmcsd_cmd(obj, CMD58, 0);
		if (sta != SMBUS_OK) {
			obj->state = SM_STATE_ERROR;
			return obj->state; // return state
		}
		obj->ocr = obj->resp.ocr;
        obj->type = (obj->ocr.ccs) ? CT_SDHC : CT_SD2;
	} else {
        /* CMD8 no response, Multi Media Card */
		retry = obj->max_retry;
		do {
            /* ACMD41 Send, HCS=0, send CMD55(APP_CMD) first */
			sta = mmcsd_cmd(obj, ACMD41, 0);
		} while (obj->resp.r1.byte != 0 && sta == SMBUS_OK && --retry != 0);
		if (obj->resp.r1.byte == 0) {
			obj->type = CT_SD1;
		} else {
			/* MMC init */
			retry = obj->max_retry;
			do {
				sta = mmcsd_cmd(obj, CMD1, 0);
				if (sta != SMBUS_OK || --retry == 0) {
					obj->state = SM_STATE_ERROR;
					return obj->state; // return state
				}
			} while (obj->resp.r1.byte != 0 && sta == SMBUS_OK
					&& --retry != 0);
			if (obj->resp.r1.byte == 0) {
				obj->type = CT_MMC;
			} else {
				obj->state = SM_STATE_ERROR;
				return obj->state; // return state
			}
		}
	}
    return SM_STATE_READY;
}

mmcsd_state_t mmcsd_cmd9_readOSD(mmcsd_obj_t *obj)
{
    mmcsd_bus_status_t sta;
	sta = mmcsd_cmd(obj, CMD9, 0);
	if (sta != SMBUS_OK) {
		obj->state = SM_STATE_ERROR;
		return obj->state;
	} else {
		sta = mmcsd_read_datablock(obj, obj->csd, 16);
		if (sta != SMBUS_OK) {
			obj->state = SM_STATE_ERROR;
			return obj->state;
		}
		bswap128(obj->csd);
	}
    return SM_STATE_READY;
}

mmcsd_state_t mmcsd_cmd10_readOID(mmcsd_obj_t *obj)
{
    mmcsd_bus_status_t sta;
	sta = mmcsd_cmd(obj, CMD10, 0);
	if (sta != SMBUS_OK) {
		obj->state = SM_STATE_ERROR;
		return obj->state;
	} else {
		sta = mmcsd_read_datablock(obj, obj->cid, 16);
		if (sta != SMBUS_OK) {
			obj->state = SM_STATE_ERROR;
			return obj->state;
		}
		bswap128(obj->cid);
	}
    return SM_STATE_READY;
}

mmcsd_state_t mmcsd_cmd17_readsingleblock(mmcsd_obj_t *obj, uint8_t *pbuf, uint64_t addr, uint16_t blocksize)
{
    int sta;
    uint16_t retry_cnt = 0;
    uint16_t crc16;
    uint8_t token = 0xff;
    
    if (obj->type == CT_SDHC) {
        blocksize = 512;
        addr /= 512;
    }
	sta = mmcsd_cmd(obj, CMD17, addr);
	if (sta != SMBUS_OK) {
		obj->state = SM_STATE_ERROR;
		return obj->state;
	} else {
        /* Pooling for a valid Data Token */
        do {
            retry_cnt++;
            sta = SPI_EXCHANGEBLOCK(obj->dev, (uint8_t *)&dummy[0], &token, 1);
        } while (sta == SMBUS_OK && token == 0xff);
        if (sta != SMBUS_OK) {
            obj->errorToken = token;
            return sta;
        }
        if (token != SD_START_SINGLE_BLOCK_READ) {
            obj->errorToken = token;
            return SM_STATE_ERROR;
        }
        /* Receiving data block (16 bytes at a time) */
        while (blocksize) {
            uint16_t readSize;
            if (blocksize > 16) {
                readSize = 16;
                blocksize -= 16;
            } else {
                readSize = blocksize;
                blocksize = 0;
            }
            sta = SPI_EXCHANGEBLOCK(obj->dev, (uint8_t *)&dummy[0], pbuf, readSize);
            if (sta != SMBUS_OK) {
                return SM_STATE_ERROR;
            }
            pbuf += 16;
        }
        /* Receive CRC but discarding it for now */
        sta = SPI_EXCHANGEBLOCK(obj->dev, (uint8_t *)&dummy[0], (uint8_t*)&crc16, 2);
        if (sta != SMBUS_OK) {
            return sta;  
        }
	}
    
    return SM_STATE_READY;
}

mmcsd_state_t mmcsd_cmd17_readmultiblocks(mmcsd_obj_t *obj, uint8_t *pbuf, uint64_t addr, uint16_t blocksize, uint32_t numblocks)
{
    int sta;
    uint16_t retry_cnt = 0;
    uint16_t crc16;
    uint8_t token;
    uint32_t offset = 0;

    while (numblocks--) {
        sta = mmcsd_cmd(obj, CMD17, addr + offset);
        if (sta != SMBUS_OK) {
            obj->state = SM_STATE_ERROR;
            return obj->state;
        } else {
            /* Pooling for a valid Data Token */
            do {
                retry_cnt++;
                sta = SPI_EXCHANGEBLOCK(obj->dev, (uint8_t *)&dummy[0], &token, 1);
            } while (sta == SMBUS_OK && token == 0xff);
            if (sta != SMBUS_OK) {
                obj->errorToken = token;
                return sta;
            }
            if (token != SD_START_SINGLE_BLOCK_READ) {
                obj->errorToken = token;
                return SM_STATE_ERROR;
            }

            uint16_t remaining = blocksize;
            uint8_t *ptr = pbuf + offset * blocksize;
            /* Receiving data block (16 bytes at a time) */
            while (remaining) {
                uint16_t readSize = (remaining > 16) ? 16 : remaining;
                sta = SPI_EXCHANGEBLOCK(obj->dev, (uint8_t *)&dummy[0], ptr, readSize);
                if (sta != SMBUS_OK)
                    return sta;
                ptr += readSize;
                remaining -= readSize;
            }
            offset += 1;
            /* Receive CRC but discarding it for now */
            sta = SPI_EXCHANGEBLOCK(obj->dev, (uint8_t *)&dummy[0], (uint8_t*)&crc16, 2);
            if (sta != SMBUS_OK)
                return sta;
        }
    }
    return SM_STATE_READY;
}

mmcsd_state_t mmcsd_cmd24_writesingleblock(mmcsd_obj_t *obj, uint8_t *pbuf, uint64_t addr, uint16_t blocksize)
{
    int sta;
    uint16_t retry_cnt = 0;
    uint16_t crc16;
    uint8_t token;
    uint8_t send_token[2] = {0xff, 0xfe};
    
    if (obj->type == CT_SDHC) {
        blocksize = 512;
        addr /= 512;
    }
	sta = mmcsd_cmd(obj, CMD24, addr);
	if (sta != SMBUS_OK) {
		obj->state = SM_STATE_ERROR;
		return obj->state;
	} else {
        sta = SPI_SNDBLOCK(obj->dev, (uint8_t *)&send_token[0], 2);
        if (sta != SMBUS_OK)
            return sta;
        sta = SPI_SNDBLOCK(obj->dev, (uint8_t *)&pbuf[0], blocksize);
        if (sta != SMBUS_OK)
            return sta; 
        sta = SPI_EXCHANGEBLOCK(obj->dev, (uint8_t *)&dummy[0], (uint8_t*)&crc16, 2);
        if (sta != SMBUS_OK)
            return sta; 
	}
    
    return SM_STATE_READY;    
}

mmcsd_state_t mmcsd_cmd24_writemultiblocks(mmcsd_obj_t *obj, uint8_t *pbuf, uint64_t addr, uint16_t blocksize, uint32_t numblocks)
{
    int sta;
    uint16_t retry_cnt = 0;
    uint16_t crc16;
    uint8_t token;
    uint8_t response;
    uint8_t send_token[2] = {0xff, 0xfe};
    uint32_t offset = 0;

    if (numblocks == 1) {
        send_token[1] = 0xfe;
    } else {
        send_token[1] = 0xfc;
    }

    while (numblocks--) {
        sta = mmcsd_cmd(obj, CMD24, addr + offset);
        if (sta != SMBUS_OK) {
            obj->state = SM_STATE_ERROR;
            return obj->state;
        } else {
            sta = SPI_SNDBLOCK(obj->dev, (uint8_t *)&send_token[1], 1);
            if (sta != SMBUS_OK)
                return sta;
            sta = SPI_SNDBLOCK(obj->dev, (uint8_t *)&pbuf[0], blocksize);
            if (sta != SMBUS_OK)
                return sta;
            sta = SPI_EXCHANGEBLOCK(obj->dev, (uint8_t *)&dummy[0], (uint8_t*)&crc16, 2);
            // read response byte
            sta = SPI_EXCHANGEBLOCK(obj->dev, (uint8_t *)&dummy[0], (uint8_t*)&response, 1);
            if ((response & 0x1f)!= 0x05) {
                return SM_STATE_ERROR;
            }

            uint8_t recv = 0;
            uint32_t timestamp = 0;
            uint32_t elapsed = 0;
            timestamp = board_get_time();
            do {
                SPI_EXCHANGEBLOCK(obj->dev, (uint8_t *)&dummy[0], &recv, 1);
                if (recv == 0xff) {
                    break;
                }
                elapsed = board_elapsed_time(timestamp);
            } while (elapsed < 500); 

            if (elapsed >= 500) {
                return SM_STATE_ERROR;
            }

            offset += 1;   // LBA 512
            pbuf += 512;
        }
    }
    return SM_STATE_READY; 
}

/***************************************
 *** public methods
***************************************/
static mmcsd_obj_t *mmcsd_spi_obj[4];

mmcsd_obj_t *get_mmcsd_spi_obj(int controller) {
    return mmcsd_spi_obj[controller];
}

void set_mmcsd_spi_obj(mmcsd_obj_t *obj, int controller) {
    mmcsd_spi_obj[controller] = obj;
}

mmcsd_state_t mmcsd_spi_init(mmcsd_obj_t *obj, struct spi_dev_s *spidev, uint8_t slotid)
{
    obj->slotid = slotid;
    obj->dev = spidev;

    if (obj->state != SM_STATE_RESET)
        return obj->state;

    obj->state = SM_STATE_BUSY;
    obj->active_lock = 1;
    mmcsd_deselect(obj);

    /**
     * Card-Identification Mode: CMD0 Reset Card
     */
    if (mmcsd_cmd0_goidlestate(obj) != SM_STATE_READY)
        return obj->state;

    /**
     * Card-Identification Mode: Operating Condition Validation
     */
    if (mmcsd_cmd8_optcondvalidation(obj) != SM_STATE_READY)
        return obj->state;

    /* At least between 10Mhz and 20Mhz for MMC or 25Mhz for SDx */
	mmcsd_select(obj);
    if(mmcsd_cmd9_readOSD(obj) != SM_STATE_READY) {
        mmcsd_deselect(obj);
        return obj->state;
    }

    if(mmcsd_cmd10_readOID(obj) != SM_STATE_READY) {
        mmcsd_deselect(obj);
        return obj->state;
    }
    mmcsd_deselect(obj);

	obj->csd_ver = obj->type == CT_MMC ? 1 : (uint8_t)unpackreg(obj->csd, CSD_VER) + 1;
	obj->blocklen_RD = (1 << (uint8_t)unpackreg(obj->csd, READ_BL_LEN));
	obj->blocklen_WR = (1 << (uint8_t)unpackreg(obj->csd, WRITE_BL_LEN));
	obj->sectorlen = (uint8_t)unpackreg(obj->csd, ERASE_SECTOR_SIZE) + 1;
	if (obj->csd_ver == 1) {
		/* (page 229) */
		obj->blockcount = ((uint16_t)unpackreg(obj->csd, C_SIZE_v1) + 1)
				* (1 << ((uint8_t)unpackreg(obj->csd, C_SIZE_MULT) + 2));
		obj->capacity = (uint64_t)obj->blockcount * (uint64_t)obj->blocklen_RD;
	} else {
		/* (page 234 / 238) */
        obj->blockcount = ((unpackreg(obj->csd, C_SIZE_v3) + 1U) * 1024);
        obj->capacity = (uint64_t)obj->blockcount * (uint64_t)obj->blocklen_RD;
	}

	obj->state = SM_STATE_READY;

    set_mmcsd_spi_obj(obj, slotid);

    MMCSPI_INFO("[board] mmcsd_spi card info v%d, blocklen:%d, sectorlen:%d, capacity:%.2fGB\r\n", 
        obj->csd_ver, obj->blocklen_RD, obj->sectorlen,
        (float)(obj->capacity)/1024.0/1024.0/1024.0);

    return obj->state;
}

mmcsd_result_t mmcsd_spi_ioctrl(mmcsd_obj_t *obj, uint8_t cmd, void *buff)
{
	mmcsd_result_t res = MMCSD_RES_OK;
	if (obj->state != SM_STATE_READY) 
        return MMCSD_RES_NOTRDY;

	switch (cmd) {
	case CTRL_POWER:
		switch (*(uint8_t*)buff) {
		case 0:
			/* Power Off */
			res = MMCSD_RES_OK;
			break;
		case 1:
			/* Power On */
			res = MMCSD_RES_OK;
			break;
		case 2:
			/* Power Check */
			res = MMCSD_RES_OK;
			break;
		default:
			res = MMCSD_RES_PARERR;
		}
		break;
	case GET_SECTOR_COUNT:
		*(uint32_t*) buff = obj->blockcount;
		break;
	case GET_SECTOR_SIZE:
		*(uint16_t*) buff = obj->blocklen_RD;
		break;
	case CTRL_SYNC:
		res = mmcsd_readywait(obj);
		break;
	case MMC_GET_CSD:
		memcpy(buff, obj->csd, 16);
		break;
	case MMC_GET_CID:
		memcpy(buff, obj->cid, 16);
		break;
	case MMC_GET_OCR:
		memcpy(buff, &obj->ocr, 4);
		break;
	default:
		res = MMCSD_RES_PARERR;
	}

	return res;
}

mmcsd_state_t mmcsd_spi_readblock(mmcsd_obj_t *obj, uint8_t *p, uint32_t addr, uint32_t blocksize)
{
    int res;
    mmcsd_select(obj);
    res = mmcsd_cmd17_readsingleblock(obj, p, addr, blocksize);
    mmcsd_deselect(obj);
    return res;
}

mmcsd_state_t mmcsd_spi_readmultiblocks(mmcsd_obj_t *obj, uint8_t *p, uint32_t addr, uint32_t blocksize, uint32_t numblock)
{
    int res;
    mmcsd_select(obj);
    res = mmcsd_cmd17_readmultiblocks(obj, p, addr, blocksize, numblock);
    mmcsd_deselect(obj);
    return res;
}

mmcsd_state_t mmcsd_spi_writeblock(mmcsd_obj_t *obj, uint8_t *p, uint32_t addr, uint32_t blocksize)
{
    int res;
    mmcsd_select(obj);
    res = mmcsd_cmd24_writesingleblock(obj, p, addr, blocksize);
    mmcsd_deselect(obj);
    return res;
}

mmcsd_state_t mmcsd_spi_writemultiblocks(mmcsd_obj_t *obj, uint8_t *p, uint32_t addr, uint32_t blocksize, uint32_t numblock)
{
    int res;
    mmcsd_select(obj);
    res = mmcsd_cmd24_writemultiblocks(obj, p, addr, blocksize, numblock);
    mmcsd_deselect(obj);
    return res;
}

/****************************************************************************
 * FatFS Support
 ****************************************************************************/
#if defined(CONFIG_FATFS_ENABLE)
#include "ff.h"
#include "ff_drv.h"

DSTATUS mmcsdspi_init(BYTE);
DSTATUS mmcsdspi_status(BYTE);
DRESULT mmcsdspi_read(BYTE, BYTE*, DWORD, UINT);
#if _USE_WRITE == 1
DRESULT mmcsdspi_write(BYTE, const BYTE*, DWORD, UINT);
#endif
#if _USE_IOCTL == 1
DRESULT mmcsdspi_ioctl(BYTE, BYTE, void*);
#endif

static mmcsd_obj_t *      fs_mmcsdspi;
const  diskio_drv_ops_t   mmcsd_spi_driver;
static FATFS              mmcsd_spi_fatfs;
static char               mmcsd_spi_mnt_path[20];

static volatile DSTATUS   mmcsd_spi_stat;
const diskio_drv_ops_t    mmcsd_spi_driver =
{
    mmcsdspi_init,
    mmcsdspi_status,
    mmcsdspi_read,
    #if _USE_WRITE == 1
    mmcsdspi_write,
    #endif
    #if _USE_IOCTL == 1
    mmcsdspi_ioctl,
    #endif
};

void hw_mmcsd_spi_fs_init(int controller)
{
    fs_mmcsdspi = get_mmcsd_spi_obj(controller);
    fatfs_link_drv(&mmcsd_spi_driver, &mmcsd_spi_mnt_path[0]);
    FRESULT ret_ff = f_mount(&mmcsd_spi_fatfs, &mmcsd_spi_mnt_path[0], 0);
    if (ret_ff != FR_OK) {
        MMCSPI_INFO("[fat] mmcsd mount failed %d\r\n", ret_ff);
        return;
    }
}

DSTATUS mmcsdspi_init(BYTE lun)
{
    uint8_t tmp;
    mmcsd_spi_stat = STA_NOINIT;
    tmp = mmcsd_readywait(fs_mmcsdspi);
    if (tmp == 0) {
        mmcsd_spi_stat &= ~STA_NOINIT;
    }
    return mmcsd_spi_stat;
}

DSTATUS mmcsdspi_status(BYTE lun)
{
    uint8_t tmp;
    mmcsd_spi_stat = STA_NOINIT;
    tmp = mmcsd_readywait(fs_mmcsdspi);
    if (tmp == 0) {
        mmcsd_spi_stat &= ~STA_NOINIT;
    }
    return mmcsd_spi_stat;
}

DRESULT mmcsdspi_read(BYTE lun, BYTE *buff, DWORD block, UINT count)
{
	DRESULT res = RES_ERROR;
	int read_status = 0;

    // blocking mode
    read_status = mmcsd_spi_readmultiblocks(fs_mmcsdspi, (uint8_t *)buff, block, fs_mmcsdspi->blocklen_RD, count); 
    // if (read_status == SM_STATE_READY) {
    //     read_status = mmcsd_readywait(fs_mmcsdspi);
    // }

    return (read_status == SM_STATE_READY) ? RES_OK : RES_ERROR;
}

#if _USE_WRITE == 1
DRESULT mmcsdspi_write(BYTE lun, const BYTE *buff, DWORD block, UINT count)
{
	DRESULT res = RES_ERROR;
	int write_status = 0;

    // blocking mode
    write_status = mmcsd_spi_writemultiblocks(fs_mmcsdspi, (uint8_t *)buff, block, fs_mmcsdspi->blocklen_RD, count);   
    // if (write_status == SM_STATE_READY) {
    //     write_status = mmcsd_readywait(fs_mmcsdspi);
    // }

    return (write_status == SM_STATE_READY) ? RES_OK : RES_ERROR;
}
#endif

#if _USE_IOCTL == 1
DRESULT mmcsdspi_ioctl(BYTE lun, BYTE cmd, void *buff)
{
	DRESULT res = RES_ERROR;

	if (mmcsd_spi_stat & STA_NOINIT) 
		return RES_NOTRDY;

	switch (cmd) {
	/* Make sure that no pending write process */
	case CTRL_SYNC :
		res = RES_OK;
		break;

	/* Get number of sectors on the disk (DWORD) */
	case GET_SECTOR_COUNT :
		*(DWORD*)buff = fs_mmcsdspi->blockcount;
		res = RES_OK;
		break;

	/* Get R/W sector size (WORD) */
	case GET_SECTOR_SIZE :
		*(WORD*)buff = fs_mmcsdspi->blocklen_RD;
		res = RES_OK;
		break;

	/* Get erase block size in unit of sector (DWORD) */
	case GET_BLOCK_SIZE :
		*(DWORD*)buff = fs_mmcsdspi->blocklen_RD;
		res = RES_OK;
		break;

	default:
		res = RES_PARERR;
	}

	return res;
}
#endif

#endif // end with macro CONFIG_FATFS_ENABLE
