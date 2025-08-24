#include <stdio.h>
#include <string.h>

#include "modbus_master.h"
#include "modbus_slave.h"


int main()
{
    uint8_t buffer[256] = {0};
    uint16_t frame_len = encode_read_request(1, 0x0258, 2, buffer, sizeof(buffer));
    if (frame_len == 0)
    {
        printf("Failed to create request\n");
        return -1;
    }

    for (int i = 0; i < frame_len; i++)
    {
        printf("%02X ", buffer[i]);
    }
    printf("\n");

    memset(buffer, 0, sizeof(buffer));

    uint16_t regs[2] = {0x03E8, 0x1388};
    frame_len = encode_read_response(1, regs, 2, buffer, sizeof(buffer));
    if (frame_len == 0)
    {
        printf("Failed to create request\n");
        return -1;
    }

    for (int i = 0; i < frame_len; i++)
    {
        printf("%02X ", buffer[i]);
    }
    printf("\n");


    uint16_t read_regs[2] = {0};
    int ret = decode_read_response(buffer, sizeof(buffer), read_regs, 2);

    for (int i = 0; i < 2; i++)
    {
        printf("%d  ", read_regs[i]);
    }
    printf("\n");

    return 0;
}