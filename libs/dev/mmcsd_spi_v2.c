#include <dev/mmcsd_spi_v2.h>

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
} sdmmc_cmd_t;

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
int sdmmc_readywait(sdmmc_obj_t *obj)
{
    return obj->hspi.sync();    
}

void sdmmc_select(sdmmc_obj_t *obj)
{
	obj->active_lock++;
	obj->hspi.active();
}

void sdmmc_deselect(sdmmc_obj_t *obj)
{
	obj->active_lock--;
	if (obj->active_lock == 0)
	    obj->hspi.disactive();
}

sdmmc_bus_status_t sdmmc_recv_r1(sdmmc_obj_t *obj)
{
	uint8_t ncr = 10;
	int sta;
	do {
        sta = obj->hspi.readwrite((uint8_t *)&dummy[0], &obj->resp.r1.byte, 1);
        if (sta != SMBUS_OK)
			break;
		if (--ncr == 0) {
			sta = SMBUS_ERROR;
			break;
		}
	} while (obj->resp.r1.start);
	return sta;
}

sdmmc_bus_status_t sdmmc_recv_r2(sdmmc_obj_t *obj)
{
	int sta;
	sta = sdmmc_recv_r1(obj);
	if (sta == SMBUS_OK) {
        sta = obj->hspi.readwrite((uint8_t *)&dummy[0], &obj->resp.r2.byte, 1);
	}
	return sta;
}

sdmmc_bus_status_t sdmmc_recv_r3_r7(sdmmc_obj_t *obj)
{
	int sta;
	uint32_t buf;
	sta = sdmmc_recv_r1(obj);
	if (sta == SMBUS_OK) {
        sta = obj->hspi.readwrite((uint8_t *)&dummy[0], (uint8_t*)&buf, 4);
		if (sta == SMBUS_OK)
			obj->resp.dword = __builtin_bswap32(buf);
	}
	return sta;
}

sdmmc_bus_status_t sdmmc_cmd(sdmmc_obj_t *obj, const command_t idx, const argument_t arg)
{
	sdmmc_cmd_t cmd;
	int sta;
	if (obj->active_lock >= 2)
		return SMBUS_BUSY;

    cmd.idx = idx;
    cmd.arg = bswap32(arg);
    // cmd.crc = get_crc7((const uint8_t*)&cmd, sizeof(sdmmc_cmd_t) - 1);
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
		sta = sdmmc_cmd(obj, CMD55, 0);
		if (sta != SMBUS_OK)
			return sta;
		//break is omitted intentionally
	default:
		/* All other commands skip CRC check (by default but can be activated) */
		cmd.crc = 0x01;
	}

    sdmmc_select(obj);
    sta = obj->hspi.write((uint8_t*) &cmd, sizeof(sdmmc_cmd_t));
	if (sta == SMBUS_OK) {
		switch (idx) {
		case CMD8:
			sta = sdmmc_recv_r3_r7(obj);
			obj->resp_type = RT_R7;
			break;
		case CMD58:
			sta = sdmmc_recv_r3_r7(obj);
			obj->resp_type = RT_R3;
			break;
		default:
			sta = sdmmc_recv_r1(obj);
			obj->resp_type = RT_R1;
		}
		if (sta == SMBUS_OK) {
			if (obj->resp.r1.byte & ~R1_IDLE)
				sta = SMBUS_ERROR;
		}
	}
    sdmmc_deselect(obj);
	return sta;
}

sdmmc_bus_status_t sdmmc_read_datablock(sdmmc_obj_t *obj, uint8_t *p, uint16_t size)
{
	uint16_t retry_cnt = 0;
	uint16_t crc16;
	int sta;
	uint8_t token;

	/* Pooling for a valid Data Token */
	do {
		retry_cnt++;
		sta = obj->hspi.readwrite((uint8_t *)&dummy[0], &token, 1);
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
		sta = obj->hspi.readwrite((uint8_t *)&dummy[0], p, readSize);
		if (sta != SMBUS_OK)
			return sta;
		p += 16;
	}
	/* Receive CRC but discarding it for now */
	sta = obj->hspi.readwrite((uint8_t *)&dummy[0], (uint8_t*)&crc16, 2);
	return sta;
}

sdmmc_state_t sdmmc_cmd0_goidlestate(sdmmc_obj_t *obj)
{
    sdmmc_bus_status_t sta;
    sta = obj->hspi.write((uint8_t *)&dummy[0], 10);
	if (sta != SMBUS_OK) {
		obj->state = SM_STATE_RESET;
		return obj->state;
	}

    /* CMD0: Reset all card into idle state */
	sta = sdmmc_cmd(obj, CMD0, 0);
	if (sta != SMBUS_OK || obj->resp.r1.byte != R1_IDLE) {
		obj->state = SM_STATE_ERROR;
		return obj->state;
	}
    return SM_STATE_READY;
}

sdmmc_state_t sdmmc_cmd8_optcondvalidation(sdmmc_obj_t *obj)
{
    sdmmc_bus_status_t sta;
    uint8_t retry;
    /* CMD8: send sd card condition and support info */
    /* support voltage 2.7~3.6v + check mode 10101010b */
	sta = sdmmc_cmd(obj, CMD8, 0x1AA);
	if (sta == SMBUS_OK) {
        /* CMD8 response, SD V2.0, capacity > 2G */
		retry = obj->max_retry;
		do {
            /* ACMD41 Send, HCS=1, send CMD55(APP_CMD) first */
			sta = sdmmc_cmd(obj, ACMD41, 0x40000000);
			if (sta != SMBUS_OK || --retry == 0) {
				obj->state = SM_STATE_ERROR;
				return obj->state; // return state
			}
		} while (obj->resp.r1.byte != 0);
        
        /* CMD58 Read OCR */
		sta = sdmmc_cmd(obj, CMD58, 0);
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
			sta = sdmmc_cmd(obj, ACMD41, 0);
		} while (obj->resp.r1.byte != 0 && sta == SMBUS_OK && --retry != 0);
		if (obj->resp.r1.byte == 0) {
			obj->type = CT_SD1;
		} else {
			/* MMC init */
			retry = obj->max_retry;
			do {
				sta = sdmmc_cmd(obj, CMD1, 0);
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

sdmmc_state_t sdmmc_cmd9_readOSD(sdmmc_obj_t *obj)
{
    sdmmc_bus_status_t sta;
	sta = sdmmc_cmd(obj, CMD9, 0);
	if (sta != SMBUS_OK) {
		obj->state = SM_STATE_ERROR;
		return obj->state;
	} else {
		sta = sdmmc_read_datablock(obj, obj->csd, 16);
		if (sta != SMBUS_OK) {
			obj->state = SM_STATE_ERROR;
			return obj->state;
		}
		bswap128(obj->csd);
	}
    return SM_STATE_READY;
}

sdmmc_state_t sdmmc_cmd10_readOID(sdmmc_obj_t *obj)
{
    sdmmc_bus_status_t sta;
	sta = sdmmc_cmd(obj, CMD10, 0);
	if (sta != SMBUS_OK) {
		obj->state = SM_STATE_ERROR;
		return obj->state;
	} else {
		sta = sdmmc_read_datablock(obj, obj->cid, 16);
		if (sta != SMBUS_OK) {
			obj->state = SM_STATE_ERROR;
			return obj->state;
		}
		bswap128(obj->cid);
	}
    return SM_STATE_READY;
}

sdmmc_state_t sdmmc_cmd17_readsingleblock(sdmmc_obj_t *obj, uint8_t *pbuf, uint64_t addr, uint16_t blocksize)
{
    int sta;
    uint16_t retry_cnt = 0;
    uint16_t crc16;
    uint8_t token = 0xff;
    
    if (obj->type == CT_SDHC) {
        blocksize = 512;
        addr /= 512;
    }
	sta = sdmmc_cmd(obj, CMD17, addr);
	if (sta != SMBUS_OK) {
		obj->state = SM_STATE_ERROR;
		return obj->state;
	} else {
        /* Pooling for a valid Data Token */
        do {
            retry_cnt++;
            sta = obj->hspi.readwrite((uint8_t *)&dummy[0], &token, 1);
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
            sta = obj->hspi.readwrite((uint8_t *)&dummy[0], pbuf, readSize);
            if (sta != SMBUS_OK) {
                return SM_STATE_ERROR;
            }
            pbuf += 16;
        }
        /* Receive CRC but discarding it for now */
        sta = obj->hspi.readwrite((uint8_t *)&dummy[0], (uint8_t*)&crc16, 2);
        if (sta != SMBUS_OK) {
            return sta;  
        }
	}
    
    return SM_STATE_READY;
}

sdmmc_state_t sdmmc_cmd17_readmultiblocks(sdmmc_obj_t *obj, uint8_t *pbuf, uint64_t addr, uint16_t blocksize, uint32_t numblocks)
{
    int sta;
    uint16_t retry_cnt = 0;
    uint16_t crc16;
    uint8_t token;
    uint32_t offset = 0;
    
    if (obj->type == CT_SDHC) {
        blocksize = 512;
        addr /= 512;
    }
    
    while (numblocks--) {
        sta = sdmmc_cmd(obj, CMD17, addr + offset);
        if (sta != SMBUS_OK) {
            obj->state = SM_STATE_ERROR;
            return obj->state;
        } else {
            /* Pooling for a valid Data Token */
            do {
                retry_cnt++;
                sta = obj->hspi.readwrite((uint8_t *)&dummy[0], &token, 1);
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
                sta = obj->hspi.readwrite((uint8_t *)&dummy[0], pbuf, readSize);
                if (sta != SMBUS_OK)
                    return sta;
                pbuf += 16;
            }
            offset += 512;
            /* Receive CRC but discarding it for now */
            sta = obj->hspi.readwrite((uint8_t *)&dummy[0], (uint8_t*)&crc16, 2);
            if (sta != SMBUS_OK)
                return sta;
        }
    }
    return SM_STATE_READY;
}

sdmmc_state_t sdmmc_cmd24_writesingleblock(sdmmc_obj_t *obj, uint8_t *pbuf, uint64_t addr, uint16_t blocksize)
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
	sta = sdmmc_cmd(obj, CMD24, addr);
	if (sta != SMBUS_OK) {
		obj->state = SM_STATE_ERROR;
		return obj->state;
	} else {
        sta = obj->hspi.write((uint8_t *)&send_token[0], 2);
        if (sta != SMBUS_OK)
            return sta;
        sta = obj->hspi.write((uint8_t *)&pbuf[0], blocksize);
        if (sta != SMBUS_OK)
            return sta; 
        sta = obj->hspi.readwrite((uint8_t *)&dummy[0], (uint8_t*)&crc16, 2);
        if (sta != SMBUS_OK)
            return sta; 
	}
    
    return SM_STATE_READY;    
}

sdmmc_state_t sdmmc_cmd24_writemultiblocks(sdmmc_obj_t *obj, uint8_t *pbuf, uint64_t addr, uint16_t blocksize, uint32_t numblocks)
{
    int sta;
    uint16_t retry_cnt = 0;
    uint16_t crc16;
    uint8_t token;
    uint8_t send_token[2] = {0xff, 0xfe};
    uint32_t offset = 0;
    
    if (obj->type == CT_SDHC) {
        blocksize = 512;
        addr /= 512;
    }
    while (numblocks--) {
        sta = sdmmc_cmd(obj, CMD24, addr + offset);
        if (sta != SMBUS_OK) {
            obj->state = SM_STATE_ERROR;
            return obj->state;
        } else {
            sta = obj->hspi.write((uint8_t *)&send_token[0], 2);
            if (sta != SMBUS_OK)
                return sta;
            sta = obj->hspi.write((uint8_t *)&pbuf[0], blocksize);
            if (sta != SMBUS_OK)
                return sta; 
            sta = obj->hspi.readwrite((uint8_t *)&dummy[0], (uint8_t*)&crc16, 2);
            offset += 512;
            pbuf += 512;
        }        
    }
    return SM_STATE_READY; 
}

/***************************************
 *** public methods
***************************************/
void sdmmc_spi_handle_register(sdmmc_obj_t *obj, void *hobj,
    void *active, void *disactive, void *sync, void *read, void *write, void *readwrite)
{
    obj->hspi.obj = hobj;
    obj->hspi.active = active;
    obj->hspi.sync = sync;
    obj->hspi.disactive = disactive;
    obj->hspi.read = read;
    obj->hspi.write = write;
    obj->hspi.readwrite = readwrite;
}

sdmmc_state_t sdmmc_spi_init(sdmmc_obj_t *obj)
{
    if (obj->state != SM_STATE_RESET)
        return obj->state;

    obj->state = SM_STATE_BUSY;
    obj->active_lock = 1;
    sdmmc_deselect(obj);
    
    /**
     * Card-Identification Mode: CMD0 Reset Card
     */
    if (sdmmc_cmd0_goidlestate(obj) != SM_STATE_READY)
        return obj->state;
    /**
     * Card-Identification Mode: Operating Condition Validation
     */
    if (sdmmc_cmd8_optcondvalidation(obj) != SM_STATE_READY)
        return obj->state;
    
    /* At least between 10Mhz and 20Mhz for MMC or 25Mhz for SDx */
	sdmmc_select(obj);
    if(sdmmc_cmd9_readOSD(obj) != SM_STATE_READY) {
        sdmmc_deselect(obj);
        return obj->state;
    }

    if(sdmmc_cmd10_readOID(obj) != SM_STATE_READY) {
        sdmmc_deselect(obj);
        return obj->state;
    }
    sdmmc_deselect(obj);
    
	obj->csd_ver = obj->type == CT_MMC ? 1 : (uint8_t)unpackreg(obj->csd, CSD_VER) + 1;
	obj->blocklen_RD = (1 << (uint8_t)unpackreg(obj->csd, READ_BL_LEN));
	obj->blocklen_WR = (1 << (uint8_t)unpackreg(obj->csd, WRITE_BL_LEN));
	obj->sectorlen = (uint8_t)unpackreg(obj->csd, ERASE_SECTOR_SIZE) + 1;
	if (obj->csd_ver == 1) {
		/* (page 229) */
		obj->blockcount = ((uint16_t)unpackreg(obj->csd, C_SIZE_v1) + 1)
				* (1 << ((uint8_t)unpackreg(obj->csd, C_SIZE_MULT) + 2));
		obj->capacity = obj->blockcount * obj->blocklen_RD;
	} else {
		/* (page 234 / 238) */
        obj->blockcount = ((unpackreg(obj->csd, C_SIZE_v3) + 1U) * 1024);
        obj->capacity = obj->blockcount * obj->blocklen_RD;
	}
    
	obj->state = SM_STATE_READY;
    return obj->state;
}

sdmmc_result_t sdmmc_ioctrl(sdmmc_obj_t *obj, uint8_t cmd, void *buff)
{
	sdmmc_result_t res = SDMMC_RES_OK;
	if (obj->state != SM_STATE_READY) 
        return SDMMC_RES_NOTRDY;

	switch (cmd) {
	case CTRL_POWER:
		switch (*(uint8_t*)buff) {
		case 0:
			/* Power Off */
			res = SDMMC_RES_OK;
			break;
		case 1:
			/* Power On */
			res = SDMMC_RES_OK;
			break;
		case 2:
			/* Power Check */
			res = SDMMC_RES_OK;
			break;
		default:
			res = SDMMC_RES_PARERR;
		}
		break;
	case GET_SECTOR_COUNT:
		*(uint32_t*) buff = obj->blockcount;
		break;
	case GET_SECTOR_SIZE:
		*(uint16_t*) buff = obj->blocklen_RD;
		break;
	case CTRL_SYNC:
		res = sdmmc_readywait(obj);
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
		res = SDMMC_RES_PARERR;
	}

	return res;
}

sdmmc_state_t sdmmc_readblock(sdmmc_obj_t *obj, uint8_t *p, uint32_t addr, uint32_t blocksize)
{
    int res;
    sdmmc_select(obj);
    res = sdmmc_cmd17_readsingleblock(obj, p, addr, blocksize);
    sdmmc_deselect(obj);
    return res;
}

sdmmc_state_t sdmmc_readmultiblocks(sdmmc_obj_t *obj, uint8_t *p, uint32_t addr, uint32_t blocksize, uint32_t numblock)
{
    int res;
    sdmmc_select(obj);
    res = sdmmc_cmd17_readmultiblocks(obj, p, addr, blocksize, numblock);
    sdmmc_deselect(obj);
    return res;
}

sdmmc_state_t sdmmc_writeblock(sdmmc_obj_t *obj, uint8_t *p, uint32_t addr, uint32_t blocksize)
{
    int res;
    sdmmc_select(obj);
    res = sdmmc_cmd24_writesingleblock(obj, p, addr, blocksize);
    sdmmc_deselect(obj);
    return res;
}

sdmmc_state_t sdmmc_writemultiblocks(sdmmc_obj_t *obj, uint8_t *p, uint32_t addr, uint32_t blocksize, uint32_t numblock)
{
    int res;
    sdmmc_select(obj);
    res = sdmmc_cmd24_writemultiblocks(obj, p, addr, blocksize, numblock);
    sdmmc_deselect(obj);
    return res;
}
