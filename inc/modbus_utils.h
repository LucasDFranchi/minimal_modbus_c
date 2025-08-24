#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "modbus_defines.h"

// convert host uint16_t to big-endian byte order
#define MODBUS_HTONS(x) (((uint16_t)(x) >> 8) | ((uint16_t)(x) << 8))

// convert host uint32_t to big-endian byte order (in case you need it)
#define MODBUS_HTONL(x) (((uint32_t)(x) >> 24) |               \
                         (((uint32_t)(x) & 0x00FF0000) >> 8) | \
                         (((uint32_t)(x) & 0x0000FF00) << 8) | \
                         ((uint32_t)(x) << 24))

uint16_t modbus_crc16(const uint8_t *buf, uint16_t len);

static inline bool is_valid_quantity(uint16_t qty)
{
    return (qty >= 1 && qty <= MODBUS_MAX_REGS);
}

static inline bool is_valid_byte_count(uint16_t byte_count)
{
    return (byte_count >= 2 && byte_count <= (MODBUS_MAX_REGS * 2));
}

static inline bool is_valid_slave_id(uint8_t slave)
{
    return (slave <= MODBUS_MAX_SLAVES);
}

static inline bool is_valid_address_range(uint16_t addr, uint16_t qty)
{
    return ((addr + qty - 1) <= 0xFFFF);
}

//add safe memcpt