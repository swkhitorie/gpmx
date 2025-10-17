#include "w25qxx_driver.h"
#include <device/qspi.h>

int w25qxx_lfs_read(const struct lfs_config *c, lfs_block_t block,
            lfs_off_t off, void *buffer, lfs_size_t size);
int w25qxx_lfs_prog(const struct lfs_config *c, lfs_block_t block,
            lfs_off_t off, const void *buffer, lfs_size_t size);
int w25qxx_lfs_erase(const struct lfs_config *c, lfs_block_t block);
int w25qxx_lfs_sync(const struct lfs_config *c);

static uint8_t lfs_buf[W25QXX_PAGESIZE];
static uint8_t lfs_read_cache_buf[W25QXX_PAGESIZE];
static uint8_t lfs_prog_cache_buf[W25QXX_PAGESIZE];
static uint8_t lfs_lookahead_buf[256];
// configuration of the filesystem is provided by this struct
const struct lfs_config w25qxx_lfs_cfg = {
    // block device operations
    .read  = w25qxx_lfs_read,
    .prog  = w25qxx_lfs_prog,
    .erase = w25qxx_lfs_erase,
    .sync  = w25qxx_lfs_sync,

    // block device configuration
    .read_size = W25QXX_PAGESIZE,
    .prog_size = W25QXX_PAGESIZE,
    .block_size = W25QXX_SECTORSIZE,
    .block_count = 2048,
    .cache_size = 256,
    .lookahead_size = 256, 
      // between 100-1000, The higher the value, the better the performance
    .block_cycles = 128,
	.read_buffer = lfs_read_cache_buf,
	.prog_buffer = lfs_prog_cache_buf,
	.lookahead_buffer = lfs_lookahead_buf,
};

static struct __w25q_qh {
    struct qspi_dev_s *bus;
} _w25q;
static struct __w25q_qh *w25qxx_handle_get()
{
    return &_w25q;
}

static bool w25qxx_wait_status1flags(uint8_t mask, uint8_t match, uint32_t timeout)
{
    struct qspi_cmdinfo_s cmdinfo;
    struct __w25q_qh *handle = w25qxx_handle_get();
    uint8_t status = 0x00;
    int ret = 0;
    int ntimes = W25QXX_WAIT_TIMESTAMP();
    bool timefail = false;

    if (!handle->bus) {
        return false;
    }

    do {

        cmdinfo.flags = QSPICMD_READDATA;
        cmdinfo.cmd = W25QXX_CMD_READSTATUS_REG1;
        cmdinfo.buflen = 1;
        cmdinfo.buffer = &status;
        ret = QSPI_COMMAND(handle->bus, &cmdinfo);

        if ((W25QXX_WAIT_TIMESTAMP() - ntimes) >= timeout) {
            timefail = true;
            break;
        }
    } while ((status & mask) != match);

	return (!timefail);
}

static bool w25qxx_write_enable()
{
    struct qspi_cmdinfo_s cmdinfo;
    struct __w25q_qh *handle = w25qxx_handle_get();
    int ret = 0;

    if (!handle->bus) {
        return false;
    }

    cmdinfo.flags = 0;
    cmdinfo.cmd = W25QXX_CMD_WRITE_ENABLE;

    ret = QSPI_COMMAND(handle->bus, &cmdinfo);
    if (ret != 0) {
        return false;
    }

    return w25qxx_wait_status1flags(W25QXX_STATUS_REG1_WEL, 0x02, 10);
}

uint32_t w25qxx_readid()
{
    struct qspi_cmdinfo_s cmdinfo;
    struct __w25q_qh *handle = w25qxx_handle_get();

    if (!handle->bus) {
        return false;
    }

    uint8_t rxdata[4];
    uint32_t id;
    int ret = 0;

    cmdinfo.flags = QSPICMD_READDATA;
    cmdinfo.cmd = W25QXX_CMD_JEDECID;
    cmdinfo.buflen = 3;
    cmdinfo.buffer = &rxdata[0];

    ret = QSPI_COMMAND(handle->bus, &cmdinfo);
    if (ret != 0) {
        return 0xffff;
    }

    id = (rxdata[0] << 16) | (rxdata[1] << 8 ) | rxdata[2];
    return id;
}

bool w25qxx_reset()
{
    struct qspi_cmdinfo_s cmdinfo;
    struct __w25q_qh *handle = w25qxx_handle_get();
    int ret = 0;

    if (!handle->bus) {
        return false;
    }

    cmdinfo.flags = 0;
    cmdinfo.cmd = W25QXX_CMD_ENABLE_RESET;
    ret = QSPI_COMMAND(handle->bus, &cmdinfo);
    if (ret != 0) {
        return false;
    }

    if (!w25qxx_wait_status1flags(W25QXX_STATUS_REG1_BUSY, 0x00, 10)) {
        return false;
    }

    cmdinfo.cmd = W25QXX_CMD_RESET_DEVICE;
    ret = QSPI_COMMAND(handle->bus, &cmdinfo);
    if (ret != 0) {
        return false;
    }

    if (!w25qxx_wait_status1flags(W25QXX_STATUS_REG1_BUSY, 0x00, 10)) {
        return false;
    }

    return true;
}

bool w25qxx_init(int nbus)
{
    struct __w25q_qh *handle = w25qxx_handle_get();
    if (!handle->bus) {
        handle->bus = qspi_bus_get(nbus);
    }

    if (!w25qxx_reset()) {
        return false;
    }

    return true;
}

uint8_t w25qxx_sector_erase(uint32_t addr)
{
    struct qspi_cmdinfo_s cmdinfo;
    struct __w25q_qh *handle = w25qxx_handle_get();
    int ret = 0;

    if (!handle->bus) {
        return 1;
    }

    cmdinfo.flags = QSPICMD_ADDRESS;
    cmdinfo.cmd = W25QXX_CMD_SECTOR_ERASE;
    cmdinfo.addrlen = 3;
    cmdinfo.addr = addr;

    if (!w25qxx_write_enable()) {
        return 2;
    }

    ret = QSPI_COMMAND(handle->bus, &cmdinfo);
    if (ret != 0) {
        return 3;
    }

    if (!w25qxx_wait_status1flags(W25QXX_STATUS_REG1_BUSY, 0x00, 80)) {
        return 4;
    }

	return 0;
}

uint8_t w25qxx_blockerase_32k(uint32_t addr)
{
    struct qspi_cmdinfo_s cmdinfo;
    struct __w25q_qh *handle = w25qxx_handle_get();
    int ret = 0;

    if (!handle->bus) {
        return 1;
    }

    cmdinfo.flags = QSPICMD_ADDRESS;
    cmdinfo.cmd = W25QXX_CMD_BLOCKERASE_32K;
    cmdinfo.addrlen = 3;
    cmdinfo.addr = addr;

    if (!w25qxx_write_enable()) {
        return 2;
    }

    ret = QSPI_COMMAND(handle->bus, &cmdinfo);
    if (ret != 0) {
        return 3;
    }

    if (!w25qxx_wait_status1flags(W25QXX_STATUS_REG1_BUSY, 0x00, 200)) {
        return 4;
    }

	return 0;
}

uint8_t w25qxx_blockerase_64k(uint32_t addr)
{
    struct qspi_cmdinfo_s cmdinfo;
    struct __w25q_qh *handle = w25qxx_handle_get();
    int ret = 0;

    if (!handle->bus) {
        return 1;
    }

    cmdinfo.flags = QSPICMD_ADDRESS;
    cmdinfo.cmd = W25QXX_CMD_BLOCKERASE_64K;
    cmdinfo.addrlen = 3;
    cmdinfo.addr = addr;

    if (!w25qxx_write_enable()) {
        return 2;
    }

    ret = QSPI_COMMAND(handle->bus, &cmdinfo);
    if (ret != 0) {
        return 3;
    }

    if (!w25qxx_wait_status1flags(W25QXX_STATUS_REG1_BUSY, 0x00, 200)) {
        return 4;
    }

	return 0;
}

uint8_t w25qxx_chiperase()
{
    struct qspi_cmdinfo_s cmdinfo;
    struct __w25q_qh *handle = w25qxx_handle_get();
    int ret = 0;

    if (!handle->bus) {
        return 1;
    }

    cmdinfo.flags = 0;
    cmdinfo.cmd = W25QXX_CMD_CHIPERASE;

    if (!w25qxx_write_enable()) {
        return 2;
    }

    ret = QSPI_COMMAND(handle->bus, &cmdinfo);
    if (ret != 0) {
        return 3;
    }

    // refer option times 20s，max 100s
    if (!w25qxx_wait_status1flags(W25QXX_STATUS_REG1_BUSY, 0x00, 30*1000)) {
        return 4;
    }

	return 0;
}

uint8_t w25qxx_writepage(uint32_t addr, uint8_t *p, uint16_t size)
{
    struct qspi_meminfo_s meminfo;
    struct __w25q_qh *handle = w25qxx_handle_get();
    int ret = 0;

    if (!handle->bus) {
        return 1;
    }

    meminfo.flags = QSPIMEM_QUADIO | QSPIMEM_WRITE;
    meminfo.dummies = 0;
    meminfo.cmd = W25QXX_CMD_QUADINPUT_PAGEPROGRAM;
    meminfo.addrlen = 3;
    meminfo.addr = addr;
    meminfo.buflen = size;
    meminfo.buffer = p;

    if (!w25qxx_write_enable()) {
        return 2;
    }

    ret = QSPI_MEMORY(handle->bus, &meminfo);
    if (ret != 0) {
        return 3;
    }

    if (!w25qxx_wait_status1flags(W25QXX_STATUS_REG1_BUSY, 0x00, 4000)) {
        return 4;
    }

	return 0;
}

uint8_t w25qxx_writebuffer(uint32_t addr, uint8_t *p, uint32_t size)
{
    uint32_t end_addr, current_size, current_addr;
    uint8_t *write_data;
    int ret = 0;

    current_size = W25QXX_PAGESIZE - (addr % W25QXX_PAGESIZE);

    if (current_size > size) {
        current_size = size;
    }

	current_addr = addr;
	end_addr = addr + size;
	write_data = p;

    do {
        ret = w25qxx_writepage(current_addr, write_data, current_size);
        if (ret != 0) {
            return ret;
        } else {
			current_addr += current_size;
			write_data += current_size;
			current_size = ((current_addr + W25QXX_PAGESIZE) > end_addr) ? 
                            (end_addr - current_addr) : W25QXX_PAGESIZE;
		}
	} while (current_addr < end_addr);

	return 0;
}

uint8_t w25qxx_readbuffer(uint32_t addr, uint8_t *p, uint32_t size)
{
    struct qspi_meminfo_s meminfo;
    struct __w25q_qh *handle = w25qxx_handle_get();
    int ret = 0;

    if (!handle->bus) {
        return 1;
    }

    meminfo.flags = QSPIMEM_QUADIO | QSPIMEM_AQUAD | QSPIMEM_READ;
    meminfo.dummies = 6;
    meminfo.cmd = W25QXX_CMD_FASTREAD_QUADIO;
    meminfo.addrlen = 3;
    meminfo.addr = addr;
    meminfo.buflen = size;
    meminfo.buffer = p;

    ret = QSPI_MEMORY(handle->bus, &meminfo);
    if (ret != 0) {
        return 2;
    }

	return 0;
}

void w25qxx_debug(int nbus)
{
    // uint8_t wdatabuff[256] = {0};
    // uint8_t rdatabuff[256] = {0};

    // bool a = w25qxx_init(nbus);
    // uint32_t id = w25qxx_readid();
    // printf("ret: %d, id:%x\r\n",a,id);

    // int t11 = w25qxx_sector_erase(0);
    // printf("erase ret: %d\r\n",t11);
    // int rt = w25qxx_readbuffer(0, rdatabuff, 16);
    // printf("read rt: %d \r\n", rt);
    // for (int i = 0;i < 16; i++) {
    //     wdatabuff[i] = i;
    //     printf("%x ", rdatabuff[i]);
    // }
    // printf("\r\n");
    // int rt2 = w25qxx_writepage(0, wdatabuff, 16);
    // printf("write rt2: %d \r\n", rt2);
    // int rt3 = w25qxx_readbuffer(0, rdatabuff, 16);
    // printf("read rt3: %d \r\n", rt3);
    // for (int i = 0;i < 16; i++) {
    //     printf("%x ", rdatabuff[i]);
    // }
    // printf("\r\n");

    // int t51 = w25qxx_sector_erase(4096);
    // printf("erase ret: %d\r\n",t51);

    // int rt5 = w25qxx_readbuffer(4096, rdatabuff, 16);
    // printf("read rt5: %d \r\n", rt5);
    // for (int i = 0;i < 16; i++) {
    //     printf("%x ", rdatabuff[i]);
    // }
    // printf("\r\n");
    // int rt4 = w25qxx_writepage(4096, wdatabuff, 16);
    // printf("write rt4: %d \r\n", rt4);
    // int rt6 = w25qxx_readbuffer(4096, rdatabuff, 16);
    // printf("read rt6: %d \r\n", rt6);
    // for (int i = 0;i < 16; i++) {
    //     printf("%x ", rdatabuff[i]);
    // }
    // printf("\r\n");
    // int rt30 = w25qxx_readbuffer(0, rdatabuff, 16);
    // printf("read rt30: %d \r\n", rt30);
    // for (int i = 0;i < 16; i++) {
    //     printf("%x ", rdatabuff[i]);
    // }
    // printf("\r\n");
    // int rt60 = w25qxx_readbuffer(4096, rdatabuff, 16);
    // printf("read rt30: %d \r\n", rt60);
    // for (int i = 0;i < 16; i++) {
    //     printf("%x ", rdatabuff[i]);
    // }
    // printf("\r\n");
}

/****************************************************************************
 * LittleFS Callback 
 ****************************************************************************/
int w25qxx_lfs_read(const struct lfs_config *c, lfs_block_t block,
            lfs_off_t off, void *buffer, lfs_size_t size)
{
    int ret = w25qxx_readbuffer((block * c->block_size + off), (uint8_t *)buffer, size);
    if (ret != 0) {
        printf("error w25qxx_lfs_read %d %x \r\n", ret, (uint32_t *)buffer);
        return LFS_ERR_IO;
    }
    return LFS_ERR_OK;
}

int w25qxx_lfs_prog(const struct lfs_config *c, lfs_block_t block,
            lfs_off_t off, const void *buffer, lfs_size_t size)
{
    int ret = w25qxx_writepage((block * c->block_size + off), (uint8_t *)buffer, size);
    if (ret != 0) {
        printf("error w25qxx_lfs_prog %d\r\n", ret);
        return LFS_ERR_IO;
    }
    return LFS_ERR_OK;
}

int w25qxx_lfs_erase(const struct lfs_config *c, lfs_block_t block)
{
    int ret = w25qxx_sector_erase(block*c->block_size);
    if (ret != 0) {
        printf("error lfs_erase %d \r\n", ret);
        return LFS_ERR_IO;
    }
    return LFS_ERR_OK;
}

int w25qxx_lfs_sync(const struct lfs_config *c)
{
    return LFS_ERR_OK;
}

struct lfs_config *w25qxx_get_lfs_ops()
{
    return &w25qxx_lfs_cfg;
}
