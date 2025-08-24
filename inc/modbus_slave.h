#pragma once

#include <stdint.h>
#include <stddef.h>

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
                              uint8_t *buffer, size_t bufsize);