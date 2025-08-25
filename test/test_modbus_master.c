#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdint.h>
#include <string.h>
#include <cmocka.h>

#include "modbus_master.h"
#include "modbus_slave.h"
#include "modbus_utils.h"
#include "modbus_types.h"

static uint8_t test_slave_id = 1;

// Helper to encode a request and capture the buffer
static uint16_t encode_request(uint8_t slave_id, uint16_t addr, uint16_t qty, uint8_t *buffer) {
    return encode_read_request(slave_id, addr, qty, buffer, 256);
}

// Helper to encode a response with given registers
static uint16_t encode_response(uint8_t slave_id, uint16_t *regs, uint8_t qty, uint8_t *buffer) {
    return encode_read_response(slave_id, regs, qty, buffer, 256);
}

static void test_encode_read_request_success(void **state) {
    (void) state;
    uint8_t buffer[256] = {0};
    uint16_t len = encode_request(test_slave_id, 0x0100, 2, buffer);

    assert_true(len > 0);
    read_holding_registers_request_st *req = (read_holding_registers_request_st *)buffer;
    assert_int_equal(req->slave_id, test_slave_id);
    assert_int_equal(req->function_code, MODBUS_READ_HOLDING_REG);
    assert_int_equal(MODBUS_HTONS(req->starting_address), 0x0100);
    assert_int_equal(MODBUS_HTONS(req->qty), 2);
}

static void test_encode_read_request_invalid(void **state) {
    (void) state;
    uint8_t buffer[256] = {0};
    // invalid quantity
    assert_int_equal(encode_request(test_slave_id, 0x0100, 0, buffer), 0);
    // invalid slave
    assert_int_equal(encode_request(250, 0x0100, 2, buffer), 0);
    // invalid address range
    assert_int_equal(encode_request(test_slave_id, 0xFFFF, 2, buffer), 0);
}

static void test_decode_read_response_success(void **state) {
    (void) state;
    uint16_t regs[2] = {1000, 5000};
    uint8_t buffer[256] = {0};
    encode_response(test_slave_id, regs, 2, buffer);

    uint16_t read_regs[2] = {0};
    int ret = decode_read_response(buffer, sizeof(buffer), read_regs, 2);

    assert_int_equal(ret, 2);
    assert_int_equal(read_regs[0], 1000);
    assert_int_equal(read_regs[1], 5000);
}

static void test_decode_read_response_errors(void **state) {
    (void) state;
    uint16_t regs[2] = {1000, 5000};
    uint8_t buffer[256] = {0};
    encode_response(test_slave_id, regs, 2, buffer);

    uint16_t read_regs[2] = {0};

    // Null pointer
    assert_int_equal(decode_read_response(NULL, sizeof(buffer), read_regs, 2), -1);
    assert_int_equal(decode_read_response(buffer, sizeof(buffer), NULL, 2), -1);

    // Wrong slave
    buffer[0] ^= 0xFF;
    assert_int_equal(decode_read_response(buffer, sizeof(buffer), read_regs, 2), -2);
    buffer[0] = test_slave_id;

    // Wrong function code
    buffer[1] = 0xFF;
    assert_int_equal(decode_read_response(buffer, sizeof(buffer), read_regs, 2), -3);
    buffer[1] = MODBUS_READ_HOLDING_REG;

    // Bad byte count
    buffer[2] = 255;
    assert_int_equal(decode_read_response(buffer, sizeof(buffer), read_regs, 2), -4);
    buffer[2] = 4;

    // Buffer too small
    assert_int_equal(decode_read_response(buffer, 2, read_regs, 2), -5);

    // Output array too small
    assert_int_equal(decode_read_response(buffer, sizeof(buffer), read_regs, 1), -6);

    // CRC mismatch
    uint16_t crc = modbus_crc16(buffer, 3 + 4);
    buffer[7] ^= 0xFF;
    assert_int_equal(decode_read_response(buffer, sizeof(buffer), read_regs, 2), -7);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_encode_read_request_success),
        cmocka_unit_test(test_encode_read_request_invalid),
        cmocka_unit_test(test_decode_read_response_success),
        cmocka_unit_test(test_decode_read_response_errors),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
