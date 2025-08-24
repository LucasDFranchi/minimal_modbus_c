#include <stdio.h>
#include <string.h>

#include "modbus_master.h"
#include "modbus_slave.h"


#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "modbus_slave.h"
#include "modbus_utils.h"

int main(void)
{
    uint8_t buffer[256] = {0};

    printf("=== Modbus Example ===\n");

    // --- Encode a Read Request ---
    uint16_t frame_len = encode_read_request(1, 0x0258, 2, buffer, sizeof(buffer));
    if (frame_len == 0)
    {
        printf("[ERROR] Failed to create Modbus Read Request\n");
        return -1;
    }

    printf("[INFO] Encoded Read Request (slave=1, addr=0x0258, qty=2):\n");
    for (int i = 0; i < frame_len; i++)
    {
        printf("%02X ", buffer[i]);
    }
    printf("\n\n");

    // --- Encode a Read Response ---
    memset(buffer, 0, sizeof(buffer));
    uint16_t regs[2] = {0x03E8, 0x1388}; // Example values: 1000 and 5000

    frame_len = encode_read_response(1, regs, 2, buffer, sizeof(buffer));
    if (frame_len == 0)
    {
        printf("[ERROR] Failed to create Modbus Read Response\n");
        return -1;
    }

    printf("[INFO] Encoded Read Response (2 registers):\n");
    for (int i = 0; i < frame_len; i++)
    {
        printf("%02X ", buffer[i]);
    }
    printf("\n\n");

    // --- Decode the Read Response ---
    uint16_t read_regs[2] = {0};
    int ret = decode_read_response(buffer, sizeof(buffer), read_regs, 2);

    if (ret < 0)
    {
        printf("[ERROR] Failed to decode response (code=%d)\n", ret);
        return -1;
    }

    printf("[INFO] Decoded %d registers:\n", ret);
    for (int i = 0; i < ret; i++)
    {
        printf("  Reg[%d] = %u (0x%04X)\n", i, read_regs[i], read_regs[i]);
    }
    printf("\n");

    printf("=== Example Finished Successfully ===\n");
    return 0;
}
