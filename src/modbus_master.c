/**
 * @file modbus_slave.c
 * @brief Implements Modbus RTU slave request encoding and response decoding.
 *
 * This module provides functions to:
 *  - Encode a read holding registers request for a Modbus slave.
 *  - Decode a read holding registers response from a Modbus slave.
 *
 * It includes validation for slave ID, register quantity, byte count, and CRC checks.
 */
#include <string.h>

#include "modbus_slave.h"
#include "modbus_utils.h"

static uint8_t last_request_slave_id = 0;

#pragma pack(push, 1)
typedef struct read_holding_registers_request_s
{
    uint8_t slave_id;               /**< Modbus slave ID */
    uint8_t function_code;          /**< Function code (0x03 for read holding registers) */
    uint16_t starting_address;      /**< Starting register address (big-endian) */
    uint16_t quantity_of_registers; /**< Number of registers to read (big-endian) */
} read_holding_registers_request_st;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct read_holding_registers_header_response_s
{
    uint8_t slave_id;      /**< Modbus slave ID in response */
    uint8_t function_code; /**< Function code in response */
    uint8_t byte_count;    /**< Number of bytes in the payload */
} read_holding_registers_header_response_st;
#pragma pack(pop)

/**
 * @brief Encode a Modbus Read Holding Registers request.
 *
 * @param slave_id Modbus slave ID (1..247)
 * @param addr Starting register address
 * @param qty Number of registers to read
 * @param buffer Output buffer to store the encoded request
 * @param bufsize Size of the output buffer
 * @return Length of the encoded request in bytes, or 0 on failure
 *
 * This function validates the input parameters and fills the buffer with the
 * Modbus RTU request frame, including CRC.
 */
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

/**
 * @brief Decode a Modbus Read Holding Registers response.
 *
 * @param buffer Buffer containing the response
 * @param bufsize Size of the buffer
 * @param regs Output array to store decoded register values
 * @param regs_len Length of the output array
 * @return Number of registers decoded on success, or a negative error code:
 *         -1: Invalid input pointers
 *         -2: Slave ID mismatch
 *         -3: Function code mismatch
 *         -4: Invalid byte count
 *         -5: Buffer too small
 *         -6: Output array too small
 *         -7: CRC mismatch
 *
 * The function validates the response header, checks CRC, and converts
 * register values from big-endian to host byte order.
 */
int decode_read_response(uint8_t *buffer, size_t bufsize,
                         uint16_t *regs, uint8_t regs_len)
{
    static const uint8_t PACKET_HEADER_SIZE = 3;
    static const uint8_t PACKET_CRC_SIZE = 2;

    if (!buffer || !regs)
    {
        return -1;
    }

    read_holding_registers_header_response_st resp = {0};
    memcpy(&resp, buffer, sizeof(resp));

    if (resp.slave_id != last_request_slave_id)
    {
        return -2;
    }

    if (resp.function_code != MODBUS_READ_HOLDING_REG)
    {
        return -3;
    }

    if (!is_valid_byte_count(resp.byte_count))
    {
        return -4;
    }

    uint16_t frame_len = PACKET_HEADER_SIZE + resp.byte_count + PACKET_CRC_SIZE;
    if (bufsize < frame_len)
    {
        return -5;
    }

    uint8_t reg_count = resp.byte_count / 2;
    if (regs_len < reg_count)
    {
        return -6;
    }

    for (int i = 0; i < reg_count; i++)
    {
        uint16_t hi = buffer[PACKET_HEADER_SIZE + (i * 2)];
        uint16_t lo = buffer[PACKET_HEADER_SIZE + (i * 2) + 1];
        regs[i] = (hi << 8) | lo;
    }

    uint16_t crc_calc = modbus_crc16(buffer, frame_len - PACKET_CRC_SIZE);
    uint16_t crc_recv = buffer[frame_len - 2] | (buffer[frame_len - 1] << 8);

    if (crc_calc != crc_recv)
    {
        return -7;
    }

    return reg_count;
}
