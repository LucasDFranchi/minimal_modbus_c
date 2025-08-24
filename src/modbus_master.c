#include <string.h>

#include "modbus_slave.h"
#include "modbus_utils.h"

static uint8_t last_request_slave_id = 0;

#pragma pack(push, 1)
typedef struct read_holding_registers_request_s
{
    uint8_t slave_id;
    uint8_t function_code;
    uint16_t starting_address;
    uint16_t quantity_of_registers;
} read_holding_registers_request_st;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct read_holding_registers_header_response_s
{
    uint8_t slave_id;
    uint8_t function_code;
    uint8_t byte_count;
} read_holding_registers_header_response_st;
#pragma pack(pop)

uint16_t encode_read_request(uint8_t slave_id, uint16_t addr, uint16_t qty, uint8_t *buffer, size_t bufsize)
{

    if ((buffer == NULL) || (bufsize < (sizeof(read_holding_registers_request_st))))
    {
        return 0;
    }

    if (!is_valid_quantity(qty) || !is_valid_slave_id(slave_id) || !is_valid_address_range(addr, qty))
    {
        return 0;
    }

    read_holding_registers_request_st r = {0};
    r.slave_id = slave_id;
    r.function_code = MODBUS_READ_HOLDING_REG;
    r.starting_address = MODBUS_HTONS(addr);
    r.quantity_of_registers = MODBUS_HTONS(qty);

    uint16_t crc = modbus_crc16((uint8_t *)&r, sizeof(r));

    memcpy(buffer, &r, sizeof(r));
    memcpy(buffer + sizeof(r), &crc, sizeof(crc));

    last_request_slave_id = slave_id;

    return sizeof(r) + sizeof(crc);
}

int decode_read_response(uint8_t *buffer, size_t bufsize,
                         uint16_t *regs, uint8_t regs_len)
{
    static const uint8_t PACKET_HEADER_SIZE = 3;
    static const uint8_t PACKET_CRC_SIZE = 2;

    if (!buffer || !regs) {
        return -1;
    }

    read_holding_registers_header_response_st resp = {0};
    memcpy(&resp, buffer, sizeof(resp));

    if (resp.slave_id != last_request_slave_id) {
        return -2;
    }

    if (resp.function_code != MODBUS_READ_HOLDING_REG) {
        return -3;
    }

    if (!is_valid_byte_count(resp.byte_count)) {
        return -4;
    }

    uint16_t frame_len = PACKET_HEADER_SIZE + resp.byte_count + PACKET_CRC_SIZE;
    if (bufsize < frame_len) {
        return -5;
    }

    uint8_t reg_count = resp.byte_count / 2;
    if (regs_len < reg_count) {
        return -6;
    }

    for (int i = 0; i < reg_count; i++) {
        uint16_t hi = buffer[PACKET_HEADER_SIZE + (i*2)];
        uint16_t lo = buffer[PACKET_HEADER_SIZE + (i*2) + 1];
        regs[i] = (hi << 8) | lo;
    }

    uint16_t crc_calc = modbus_crc16(buffer, frame_len - PACKET_CRC_SIZE);
    uint16_t crc_recv = buffer[frame_len - 2] | (buffer[frame_len - 1] << 8);

    if (crc_calc != crc_recv) {
        return -7;
    }

    return reg_count;
}
