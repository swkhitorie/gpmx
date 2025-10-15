#include "w25qxx_driver.h"
#include <device/qspi.h>

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
    uint8_t status;
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

        if (W25QXX_WAIT_TIMESTAMP() - ntimes >= timeout) {
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
        return 2;
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

    meminfo.flags = QSPIMEM_QUADIO | QSPIMEM_READ;
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

