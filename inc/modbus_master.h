#pragma once

#include <stdint.h>

uint16_t encode_read_request(uint8_t slave_id, uint16_t addr, uint16_t qty, uint8_t *buffer, size_t bufsize);

int decode_read_response(uint8_t *buffer, size_t bufsize, uint16_t *regs, uint8_t regs_len);