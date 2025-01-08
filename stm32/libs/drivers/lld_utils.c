
#include "lld_utils.h"
#include <stddef.h>

/*! @brief union for Convert memory
 *
 */
union Convert
{
    double data_double; /*!< tmp value for convert */
    float data_float[2];
    int64_t data_s64;
    int32_t data_s32[2];
    uint32_t data_u32[2];
    int16_t data_s16[4];
    uint16_t data_u16[4];
    uint8_t data_array[8];
} data_converter = { 0.0000000000000000 };

void double_to_bytes(double data, uint8_t array[], bool reverse)
{
    data_converter.data_double = data;
    for (uint8_t i = 0; i < 8; i++)
    {
        if (reverse)
            array[i] = data_converter.data_array[7 - i];
        else
            array[i] = data_converter.data_array[i];
    }
}

void float_to_bytes(float data, uint8_t array[], bool reverse)
{
    data_converter.data_float[0] = data;
    for (uint8_t i = 0; i < 4; i++)
    {
        if (reverse)
            array[i] = data_converter.data_array[3 - i];
        else
            array[i] = data_converter.data_array[i];
    }
}

void s64_to_bytes(int64_t data, uint8_t array[], bool reverse)
{
    data_converter.data_s64 = data;
    for (uint8_t i = 0; i < 8; i++)
    {
        if (reverse)
            array[i] = data_converter.data_array[7 - i];
        else
            array[i] = data_converter.data_array[i];
    }
}

void s32_to_bytes(int32_t data, uint8_t array[], bool reverse)
{
    data_converter.data_s32[0] = data;
    for (uint8_t i = 0; i < 4; i++)
    {
        if (reverse)
            array[i] = data_converter.data_array[3 - i];
        else
            array[i] = data_converter.data_array[i];
    }
}

void s16_to_bytes(int16_t data, uint8_t array[], bool reverse)
{
    data_converter.data_s16[0] = data;
    for (uint8_t i = 0; i < 2; i++)
    {
        if (reverse)
            array[i] = data_converter.data_array[1 - i];
        else
            array[i] = data_converter.data_array[i];
    }
}

void u32_to_bytes(uint32_t data, uint8_t array[], bool reverse)
{
    data_converter.data_u32[0] = data;
    for (uint8_t i = 0; i < 4; i++)
    {
        if (reverse)
            array[i] = data_converter.data_array[3 - i];
        else
            array[i] = data_converter.data_array[i];
    }
}

void u16_to_bytes(uint16_t data, uint8_t array[], bool reverse)
{
    data_converter.data_u16[0] = data;
    for (uint8_t i = 0; i < 2; i++)
    {
        if (reverse)
            array[i] = data_converter.data_array[1 - i];
        else
            array[i] = data_converter.data_array[i];
    }
}

double bytes_to_double(const uint8_t array[], bool reverse)
{
    for (uint8_t i = 0; i < 8; i++)
    {
        if (reverse)
            data_converter.data_array[i] = array[7 - i];
        else
            data_converter.data_array[i] = array[i];
    }
    return data_converter.data_double;
}

float bytes_to_float(const uint8_t array[], bool reverse)
{
    for (uint8_t i = 0; i < 4; i++)
    {
        if (reverse)
            data_converter.data_array[i] = array[3 - i];
        else
            data_converter.data_array[i] = array[i];
    }
    return data_converter.data_float[0];
}

int64_t bytes_to_s64(const uint8_t array[], bool reverse)
{
    for (uint8_t i = 0; i < 8; i++)
    {
        if (reverse)
            data_converter.data_array[i] = array[7 - i];
        else
            data_converter.data_array[i] = array[i];
    }
    return data_converter.data_s64;
}

int32_t bytes_to_s32(const uint8_t array[], bool reverse)
{
    for (uint8_t i = 0; i < 4; i++)
    {
        if (reverse)
            data_converter.data_array[i] = array[3 - i];
        else
            data_converter.data_array[i] = array[i];
    }
    return data_converter.data_s32[0];
}

int16_t bytes_to_s16(const uint8_t array[], bool reverse)
{
    for (uint8_t i = 0; i < 2; i++)
    {
        if (reverse)
            data_converter.data_array[i] = array[1 - i];
        else
            data_converter.data_array[i] = array[i];
    }
    return data_converter.data_s16[0];
}

uint32_t bytes_to_u32(const uint8_t array[], bool reverse)
{
    for (uint8_t i = 0; i < 4; i++)
    {
        if (reverse)
            data_converter.data_array[i] = array[3 - i];
        else
            data_converter.data_array[i] = array[i];
    }
    return data_converter.data_u32[0];
}

uint16_t bytes_to_u16(const uint8_t array[], bool reverse)
{
    for (uint8_t i = 0; i < 2; i++)
    {
        if (reverse)
            data_converter.data_array[i] = array[1 - i];
        else
            data_converter.data_array[i] = array[i];
    }
    return data_converter.data_u16[0];
}

uint16_t devbuf_size(devbuf_t *obj) { return obj->size; }
uint16_t devbuf_free(devbuf_t *obj) { return obj->capacity - obj->size; }
uint16_t devbuf_overflow(devbuf_t *obj) { return obj->overflow; }
bool devbuf_is_empty(devbuf_t *obj) { return (obj->size == 0); }
bool devbuf_is_full(devbuf_t *obj) { return (obj->size == obj->capacity); }
void devbuf_reset(devbuf_t *obj) { obj->size = obj->idx_out = obj->idx_in = obj->overflow = 0; } 
void devbuf_init(devbuf_t *obj, uint8_t *p, uint16_t capacity)
{
	if (p == NULL) return;
	obj->buf = p;
	obj->capacity = capacity;
	devbuf_reset(obj);
}

uint16_t devbuf_write(devbuf_t *obj, const uint8_t *p, uint16_t len)
{
	uint16_t i;
	uint16_t write_len = 0;
	uint16_t fifo_res_size = 0;

	fifo_res_size = obj->capacity - devbuf_size(obj);
	if (fifo_res_size >= len) {
		write_len = len;
	} else {
		write_len = fifo_res_size;
		obj->overflow = len - fifo_res_size;
	}

	for (i = 0; i < write_len; i++) {
		obj->buf[obj->idx_in] = p[i];
		obj->idx_in++;
		if (obj->idx_in >= obj->capacity) {
			obj->idx_in -= obj->capacity;
		}
	}

	obj->size += write_len;
	return write_len;
}

uint16_t devbuf_read(devbuf_t *obj, uint8_t *p, uint16_t len)
{
	uint16_t i;
	uint16_t read_len = 0;

	if (devbuf_size(obj) >= len) {
		read_len = len;
	} else {
		read_len = devbuf_size(obj);
	}

	for (i = 0; i < read_len; i++) {
		p[i] = obj->buf[obj->idx_out];
		obj->idx_out++;
		if (obj->idx_out >= obj->capacity) {
			obj->idx_out -= obj->capacity;
		}
	}

	obj->size -= read_len;
	return read_len;
}

uint16_t devbuf_query(devbuf_t *obj, uint16_t offset, uint8_t *p, uint16_t len)
{
	uint16_t i;
	uint16_t query_len = 0;
	uint16_t query_idx = obj->idx_out + offset;

	if (devbuf_size(obj) >= (offset + len)) {
		query_len = len;
	} else {
		query_len = devbuf_size(obj);
	}

	for (i = 0; i < query_len; i++) {
		p[i] = obj->buf[query_idx];
		query_idx++;
		if (query_idx >= obj->capacity) {
			query_idx -= obj->capacity;
		}
	}

	return query_len;
}

