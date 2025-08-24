#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "modbus_slave.h"
#include "modbus_master.h"
#include "modbus_utils.h"

int main(void)
{
    uint8_t buffer[256] = {0};

    printf("=== Modbus Example ===\n");

    // --- Encode a Read Request ---
    uint16_t req_addr = 0x0258;
    uint16_t req_qty = 2;
    uint8_t slave_id = 1;

    set_device_slave_id(slave_id);

    uint16_t frame_len = encode_read_request(slave_id, req_addr, req_qty, buffer, sizeof(buffer));
    if (frame_len == 0)
    {
        printf("[ERROR] Failed to create Modbus Read Request\n");
        return -1;
    }

    printf("[INFO] Encoded Read Request (slave=%u, addr=0x%04X, qty=%u):\n", slave_id, req_addr, req_qty);
    for (int i = 0; i < frame_len; i++)
    {
        printf("%02X ", buffer[i]);
    }
    printf("\n\n");

    // --- Decode the Read Request ---
    uint16_t recv_req_addr = 0;
    uint16_t recv_req_qty = 0;
    uint8_t recv_slave_id = 0;
    int err = decode_read_request(buffer, sizeof(buffer), &recv_slave_id, &recv_req_addr, &recv_req_qty);
    if (err != 0)
    {
        printf("[ERROR] Failed to decode Modbus Read Request: %d\n", err);
        return -1;
    }

    printf("[INFO] Decoded Read Request: slave=%u, starting_address=0x%04X, qty=%u\n\n",
           recv_slave_id, recv_req_addr, recv_req_qty);

    // --- Encode a Read Response ---
    memset(buffer, 0, sizeof(buffer));
    uint16_t regs[2] = {0x03E8, 0x1388};

    frame_len = encode_read_response(slave_id, regs, 2, buffer, sizeof(buffer));
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
