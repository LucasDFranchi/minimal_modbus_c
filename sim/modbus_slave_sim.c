#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "modbus_slave.h"
#include "modbus_utils.h"

#define PORT 5020
#define BUFFER_SIZE 256

int main() {
    int sockfd, connfd;
    struct sockaddr_in servaddr, cliaddr;
    socklen_t len = sizeof(cliaddr);
    uint8_t buffer[BUFFER_SIZE];

    // Set device slave ID
    set_device_slave_id(1);

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) { perror("socket"); return -1; }

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(PORT);

    if (bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
        perror("bind"); return -1;
    }

    listen(sockfd, 1);
    printf("[SLAVE] Listening on port %d...\n", PORT);

    connfd = accept(sockfd, (struct sockaddr*)&cliaddr, &len);
    if (connfd < 0) { perror("accept"); return -1; }

    while (1) {
        ssize_t n = read(connfd, buffer, BUFFER_SIZE);
        if (n <= 0) break;

        uint8_t slave_id;
        uint16_t start_addr, qty;
        int ret = decode_read_request(buffer, n, &slave_id, &start_addr, &qty);
        if (ret != 0) {
            printf("[SLAVE] Invalid request (ret=%d)\n", ret);
            continue;
        }

        printf("[SLAVE] Received request: start=%u qty=%u\n", start_addr, qty);

        uint16_t regs[MODBUS_MAX_REGS];
        for (int i = 0; i < qty; i++)
            regs[i] = start_addr + i; // dummy data

        uint16_t resp_len = encode_read_response(slave_id, regs, qty, buffer, BUFFER_SIZE);
        write(connfd, buffer, resp_len);
        printf("[SLAVE] Sent response (%u bytes)\n", resp_len);
    }

    close(connfd);
    close(sockfd);
    return 0;
}
