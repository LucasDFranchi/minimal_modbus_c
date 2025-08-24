#pragma once

#include <stdint.h>
#include <stddef.h>

uint16_t encode_read_response(uint8_t slave_id,
                              const uint16_t *regs, uint16_t qty,
                              uint8_t *buffer, size_t bufsize);