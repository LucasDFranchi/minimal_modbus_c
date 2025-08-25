#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "modbus_master.h"
#include "modbus_utils.h"

#define PORT 5020
#define BUFFER_SIZE 256

int main() {
    int sockfd;
    struct sockaddr_in servaddr;
    uint8_t buffer[BUFFER_SIZE];

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) { perror("socket"); return -1; }

    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &servaddr.sin_addr);

    if (connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
        perror("connect"); return -1;
    }

    uint16_t start_addr = 100;
    uint16_t qty = 5;
    uint16_t frame_len = encode_read_request(1, start_addr, qty, buffer, BUFFER_SIZE);

    write(sockfd, buffer, frame_len);
    printf("[MASTER] Sent request: start=%u qty=%u\n", start_addr, qty);

    ssize_t n = read(sockfd, buffer, BUFFER_SIZE);
    if (n <= 0) { perror("read"); return -1; }

    uint16_t read_regs[MODBUS_MAX_REGS];
    int ret = decode_read_response(buffer, n, read_regs, qty);
    if (ret < 0) {
        printf("[MASTER] Failed to decode response (ret=%d)\n", ret);
        return -1;
    }

    printf("[MASTER] Received %d registers:\n", ret);
    for (int i = 0; i < ret; i++)
        printf("  Reg[%d] = %u\n", i, read_regs[i]);

    close(sockfd);
    return 0;
}
