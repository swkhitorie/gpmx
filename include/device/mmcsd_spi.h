#ifndef MMCSD_SPI_V2_H_
#define MMCSD_SPI_V2_H_

#include <stdint.h>

/* r1 response flags */
#define R1_IDLE         0x01U   /* in idle state */
#define R1_ERASE_RES    0x02U   /* erase reset */
#define R1_CMD_ERR      0x04U   /* illegal command */
#define R1_CRC_ERR      0x08U   /* Command crc error */
#define R1_ERASE_ERR    0x10U   /* erase qequence error */
#define R1_ADDR_ERR     0x20U   /* address error */
#define R1_PARAM_ERR    0x40U   /* parameter error */
#define R1_START        0x80U   /* start bit 0*/

/* Generic command (Used by FatFs) */
#define CTRL_SYNC		0	/* Complete pending write process (needed at _FS_READONLY == 0) */
#define GET_SECTOR_COUNT	1	/* Get media size (needed at _USE_MKFS == 1) */
#define GET_SECTOR_SIZE		2	/* Get sector size (needed at _MAX_SS != _MIN_SS) */
#define GET_BLOCK_SIZE		3	/* Get erase block size (needed at _USE_MKFS == 1) */
//#define CTRL_TRIM		4	/* Inform device that the data on the block of sectors is no longer used (needed at _USE_TRIM == 1) */

/* Generic command (Not used by FatFs) */
#define CTRL_POWER			5	/* Get/Set power status */
#define CTRL_LOCK			6	/* Lock/Unlock media removal */
#define CTRL_EJECT			7	/* Eject media */
#define CTRL_FORMAT			8	/* Create physical format on the media */

/* MMC/SDC specific ioctl command */
#define MMC_GET_TYPE		10	/* Get card type */
#define MMC_GET_CSD			11	/* Get CSD */
#define MMC_GET_CID			12	/* Get CID */
#define MMC_GET_OCR			13	/* Get OCR */
#define MMC_GET_SDSTAT		14	/* Get SD status */

/* ATA/CF specific ioctl command */
//#define ATA_GET_REV			20	/* Get F/W revision */
//#define ATA_GET_MODEL		21	/* Get model name */
//#define ATA_GET_SN			22	/* Get serial number */

/**
* Start Data tokens:
* Tokens (necessary because at nop/idle (and CS active) only 0xff is 
* on the data/command line)  
*/
#define SD_START_SINGLE_BLOCK_READ    0xFE  /*!< Data token start byte, Start Single Block Read */
#define SD_START_MULTIPLE_BLOCK_READ  0xFE  /*!< Data token start byte, Start Multiple Block Read */
#define SD_START_SINGLE_BLOCK_WRITE   0xFE  /*!< Data token start byte, Start Single Block Write */
#define SD_START_MULTIPLE_BLOCK_WRITE 0xFD  /*!< Data token start byte, Start Multiple Block Write */
#define SD_STOP_MULTIPLE_BLOCK_WRITE  0xFD  /*!< Data toke stop byte, Stop Multiple Block Write */

typedef enum {
	CT_UNKNOWN = 0U,
	CT_MMC = 1U, /* MMC ver 3 */
	CT_SD1 = 2U, /* SD ver 1 */
	CT_SD2 = 3U, /* SD ver 2 */
	CT_SDHC = 5U, /* High Capacity or Extended Capacity (Block addressing) */
	CT_SDUC = 6U /* Ultra Capacity: (Block Addressing) */
} sdmmc_cardtype_t;

typedef enum {
	SM_STATE_RESET = 0U,
	SM_STATE_READY = 1U,
	SM_STATE_BUSY  = 2U,
	SM_STATE_ERROR = 3U
} sdmmc_state_t;

typedef enum {
	SMBUS_OK = 0U,
	SMBUS_ERROR = 1U,
	SMBUS_BUSY = 2U
} sdmmc_bus_status_t;

typedef enum {
	SDMMC_RES_OK = 0,
	SDMMC_RES_ERROR,
	SDMMC_RES_WRPRT,
	SDMMC_RES_NOTRDY,
	SDMMC_RES_PARERR
} sdmmc_result_t;

typedef union __attribute__((__packed__)) {
	struct {
		uint8_t idle :1; /* In Idle State */
		uint8_t erase_reset :1; /* Erase Reset */
		uint8_t cmd_err :1; /* Illegal Command */
		uint8_t crc_err :1; /* Command CRC Error */
		uint8_t erase_seq_err :1; /* Erase Sequence Error */
		uint8_t addr_err :1; /* Address Error */
		uint8_t param_err :1; /* Parameter Error */
		uint8_t start :1; /* Always 0 in a valid response*/
	};
	uint8_t byte;
} sdmmc_resp_r1_t;

typedef union __attribute__((__packed__)) {
	struct {
		uint8_t locked :1; /* Card is locked */
		uint8_t era_loc_fail :1; /* WP erase skip | lock/unlock cmd failed */
		uint8_t error :1; /* ? */
		uint8_t cc_err :1; /* ? */
		uint8_t ecc_fail :1; /* Card ECC failed */
		uint8_t wp_viol :1; /* WP violation */
		uint8_t erase_param :1; /* Erase param */
		uint8_t oor :1; /* Out of range */
	};
	uint8_t byte;
} sdmmc_resp_r2_t;

typedef struct __attribute__((__packed__)) {
	uint16_t :15; /* reserved */
	uint8_t v27_28a :1; /* Vcc 2.7-2.8V accepted */
	uint8_t v28_29a :1; /* Vcc 2.8-2.9V accepted */
	uint8_t v29_30a :1; /* Vcc 2.9-3.0V accepted */
	uint8_t v30_31a :1; /* Vcc 3.0-3.1V accepted */
	uint8_t v31_32a :1; /* Vcc 3.1-3.2V accepted */
	uint8_t v32_33a :1; /* Vcc 3.2-3.3V accepted */
	uint8_t v33_34a :1; /* Vcc 3.3-3.4V accepted */
	uint8_t v34_35a :1; /* Vcc 3.4-3.5V accepted */
	uint8_t v35_36a :1; /* Vcc 3.5-3.6V accepted */
	uint8_t s18a :1; /* Switching to 1.8V Accepted */
	uint8_t :2; /* reserved */
	uint8_t co2t :1; /* Over 2TB support status */
	uint8_t :1; /* reserved */
	uint8_t UHSIICS :1; /* UHS-II Card Status */
	uint8_t ccs :1; /* Card Capacity Status */
	uint8_t busy :1;
} sdmmc_reg_ocr_t;

typedef struct __attribute__((__packed__)) {
	sdmmc_resp_r1_t r1;
	union {
		sdmmc_resp_r2_t r2;
		sdmmc_reg_ocr_t ocr;
		struct {
			uint8_t check_pattern;
			uint8_t voltage_accepted :4;
			uint16_t :16;
			uint8_t command_version :4;
		} r7;
		uint32_t dword;
	};
} sdmmc_resp_t;

typedef enum {
	RT_R1 = 1U, /* R1b also handled by SDMMC_receive_R1 */
//	RT_R2 = 2U,   /* not used by the currently supported command set */
	RT_R3 = 3U, /* only used by CMD58 */
	RT_R7 = 7U, /* only used by CMD8 */
} sdmmc_resp_type_t;

typedef struct {
    void *obj;
    void (*active)();
    void (*disactive)();
    int (*sync)();
    int (*read)(uint8_t *p, uint16_t len);
    int (*write)(uint8_t *p, uint16_t len);
    int (*readwrite)(uint8_t *pt, uint8_t *pr, uint16_t len);
} sdmmc_spiobj_t;

typedef struct {
    sdmmc_spiobj_t hspi;
    sdmmc_resp_t resp;
    sdmmc_resp_type_t resp_type;
    sdmmc_state_t state;
    uint8_t active_lock;
	uint8_t max_retry;
	uint8_t errorToken; /* Last error token returned by a data transfer */
	sdmmc_reg_ocr_t ocr;
	sdmmc_cardtype_t type;
	uint8_t cid[16]; /* Card Identification Register (page 224) */
//	uint16_t RCA; /* Relative Card Address Register */
//	uint16_t DSR; /* Driver Stage Register */
	uint8_t csd[16]; /* Card Specific Data (page 225) */
	uint8_t csd_ver; /* CSD register version */
	uint32_t blockcount; /* SDMMC memory capacity in blocks */
	uint16_t blocklen_RD; /* Size of transfer data blocks and also the default logical sector size */
	uint16_t blocklen_WR; /* Same as blocklen_RD but for write transfers */
	uint64_t capacity; /* SDMMC memory capacity in bytes */
	uint8_t sectorlen; /* Size of an erasable sector in blocks */
} sdmmc_obj_t;

#if defined(__cplusplus)
extern "C"{
#endif

void sdmmc_spi_handle_register(sdmmc_obj_t *obj, void *hobj,
    void *active, void *disactive, void *sync, void *read, void *write, void *readwrite);

sdmmc_state_t sdmmc_spi_init(sdmmc_obj_t *obj);
sdmmc_result_t sdmmc_ioctrl(sdmmc_obj_t *obj, uint8_t cmd, void *buff);
sdmmc_state_t sdmmc_readblock(sdmmc_obj_t *obj, uint8_t *p, uint32_t addr, uint32_t blocksize);
sdmmc_state_t sdmmc_readmultiblocks(sdmmc_obj_t *obj, uint8_t *p, uint32_t addr, uint32_t blocksize, uint32_t numblock);
sdmmc_state_t sdmmc_writeblock(sdmmc_obj_t *obj, uint8_t *p, uint32_t addr, uint32_t blocksize);
sdmmc_state_t sdmmc_writemultiblocks(sdmmc_obj_t *obj, uint8_t *p, uint32_t addr, uint32_t blocksize, uint32_t numblock);

#if defined(__cplusplus)
}
#endif

#endif
