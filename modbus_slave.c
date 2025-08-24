#include <string.h>

#include "modbus_slave.h"
#include "modbus_utils.h"

#pragma pack(push, 1)
typedef struct read_holding_registers_response_s
{
    uint8_t slave_id;
    uint8_t function_code; // 0x03
    uint8_t byte_count;
    uint16_t registers[MODBUS_MAX_REGS];
} read_holding_registers_response_st;
#pragma pack(pop)

uint16_t encode_read_response(uint8_t slave_id,
                              const uint16_t *regs, uint16_t qty,
                              uint8_t *buffer, size_t bufsize)
{
    static const uint8_t PACKET_HEADER_SIZE = 3;

    if ((!buffer) || (!regs))
    {
        return 0;
    }

    if (!is_valid_quantity(qty) || !is_valid_slave_id(slave_id))
    {
        return 0;
    }

    size_t frame_len = PACKET_HEADER_SIZE + (qty * sizeof(uint16_t));
    if (bufsize < frame_len)
    {
        return 0;
    }

    read_holding_registers_response_st resp = {0};
    resp.slave_id = slave_id;
    resp.function_code = MODBUS_READ_HOLDING_REG;
    resp.byte_count = qty * sizeof(uint16_t);

    for (int i = 0; i < qty; i++)
    {
        resp.registers[i] = MODBUS_HTONS(regs[i]);
    }

    uint16_t crc = modbus_crc16((uint8_t *)&resp, 3 + (qty * 2));

    memcpy(buffer, &resp, frame_len);
    memcpy(buffer + frame_len, &crc, sizeof(crc));

    return frame_len + sizeof(crc);
}