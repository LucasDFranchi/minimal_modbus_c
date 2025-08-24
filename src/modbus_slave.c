/**
 * @file modbus_slave.c
 * @brief Encode Modbus Read Holding Registers responses for RTU communication.
 *
 * This module provides functions to encode a Modbus slave response frame,
 * including payload and CRC. It handles conversion of register values to
 * big-endian format and validates input parameters.
 */
#include <string.h>

#include "modbus_slave.h"
#include "modbus_utils.h"

#pragma pack(push, 1)
typedef struct read_holding_registers_response_s
{
    uint8_t slave_id;        /**< Modbus slave ID */
    uint8_t function_code;   /**< Function code (0x03 for read holding registers) */
    uint8_t byte_count;      /**< Number of data bytes following header */
    uint16_t registers[MODBUS_MAX_REGS]; /**< Register values in big-endian format */
} read_holding_registers_response_st;
#pragma pack(pop)

/**
 * @brief Encode a Modbus Read Holding Registers response frame.
 *
 * @param slave_id Modbus slave ID (1..247)
 * @param regs Array of register values to include in response
 * @param qty Number of registers to include (must be <= MODBUS_MAX_REGS)
 * @param buffer Output buffer to store the encoded response
 * @param bufsize Size of the output buffer
 * @return Length of the encoded response in bytes (including CRC), or 0 on failure
 *
 * This function validates the input parameters, converts registers to big-endian
 * format, and appends the CRC16 checksum at the end of the frame.
 */
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