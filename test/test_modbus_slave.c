#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdint.h>
#include <string.h>
#include <cmocka.h>

#include "modbus_slave.h"
#include "modbus_utils.h"
#include "modbus_types.h"

static uint8_t test_slave_id = 1;

// Helper to create a valid read request buffer
static void fill_valid_read_request(uint8_t *buffer, uint16_t addr, uint16_t qty, uint8_t slave_id) {
    read_holding_registers_request_st req = {0};
    req.slave_id = slave_id;
    req.function_code = MODBUS_READ_HOLDING_REG;
    req.starting_address = MODBUS_HTONS(addr);
    req.qty = MODBUS_HTONS(qty);
    memcpy(buffer, &req, sizeof(req));
    uint16_t crc = modbus_crc16(buffer, sizeof(req));
    memcpy(buffer + sizeof(req), &crc, sizeof(crc));
}

static void test_decode_read_request_success(void **state) {
    (void) state;
    uint8_t buffer[10] = {0};
    fill_valid_read_request(buffer, 0x0258, 2, test_slave_id);

    uint8_t slave = 0;
    uint16_t start_addr = 0;
    uint16_t qty = 0;
    int ret = decode_read_request(buffer, sizeof(buffer), &slave, &start_addr, &qty);

    assert_int_equal(ret, 0);
    assert_int_equal(slave, test_slave_id);
    assert_int_equal(start_addr, 0x0258);
    assert_int_equal(qty, 2);
}

static void test_decode_read_request_null_ptr(void **state) {
    (void) state;
    uint8_t buffer[10] = {0};
    uint8_t slave = 0;
    uint16_t start_addr = 0;
    uint16_t qty = 0;

    assert_int_equal(decode_read_request(NULL, sizeof(buffer), &slave, &start_addr, &qty), -1);
    assert_int_equal(decode_read_request(buffer, sizeof(buffer), NULL, &start_addr, &qty), -1);
    assert_int_equal(decode_read_request(buffer, sizeof(buffer), &slave, NULL, &qty), -1);
    assert_int_equal(decode_read_request(buffer, sizeof(buffer), &slave, &start_addr, NULL), -1);
}

static void test_decode_read_request_small_buffer(void **state) {
    (void) state;
    uint8_t buffer[5] = {0};
    uint8_t slave = 0;
    uint16_t start_addr = 0;
    uint16_t qty = 0;
    assert_int_equal(decode_read_request(buffer, sizeof(buffer), &slave, &start_addr, &qty), -2);
}

static void test_decode_read_request_bad_crc(void **state) {
    (void) state;
    uint8_t buffer[10] = {0};
    fill_valid_read_request(buffer, 0x0258, 2, test_slave_id);
    // Corrupt one byte
    buffer[2] ^= 0xFF;

    uint8_t slave = 0;
    uint16_t start_addr = 0;
    uint16_t qty = 0;
    assert_int_equal(decode_read_request(buffer, sizeof(buffer), &slave, &start_addr, &qty), -6);
}

static void test_decode_read_request_wrong_function(void **state) {
    (void) state;
    uint8_t buffer[10] = {0};
    fill_valid_read_request(buffer, 0x0258, 2, test_slave_id);
    buffer[1] = 0xFF; // invalid function code
    uint16_t crc = modbus_crc16(buffer, sizeof(read_holding_registers_request_st));
    memcpy(buffer + sizeof(read_holding_registers_request_st), &crc, sizeof(crc));

    uint8_t slave = 0;
    uint16_t start_addr = 0;
    uint16_t qty = 0;
    assert_int_equal(decode_read_request(buffer, sizeof(buffer), &slave, &start_addr, &qty), -5);
}

static void test_set_device_slave_id(void **state) {
    (void) state;
    assert_int_equal(set_device_slave_id(5), 0);
    assert_int_equal(set_device_slave_id(0), -1); // invalid
    assert_int_equal(set_device_slave_id(248), -1); // invalid
    assert_int_equal(set_device_slave_id(BROADCAST_SLAVE_ID), -1); // invalid
}

static void test_encode_read_response_success(void **state) {
    (void) state;
    uint16_t regs[2] = {1000, 5000};
    uint8_t buffer[256] = {0};
    uint16_t len = encode_read_response(test_slave_id, regs, 2, buffer, sizeof(buffer));
    assert_true(len > 0);

    // Decode back to check registers
    read_holding_registers_header_response_st *resp = (read_holding_registers_header_response_st *)buffer;
    assert_int_equal(resp->slave_id, test_slave_id);
    assert_int_equal(resp->function_code, MODBUS_READ_HOLDING_REG);
    assert_int_equal(resp->byte_count, 4);

    uint16_t r0 = (buffer[3] << 8) | buffer[4];
    uint16_t r1 = (buffer[5] << 8) | buffer[6];
    assert_int_equal(r0, 1000);
    assert_int_equal(r1, 5000);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_decode_read_request_success),
        cmocka_unit_test(test_decode_read_request_null_ptr),
        cmocka_unit_test(test_decode_read_request_small_buffer),
        cmocka_unit_test(test_decode_read_request_bad_crc),
        cmocka_unit_test(test_decode_read_request_wrong_function),
        cmocka_unit_test(test_set_device_slave_id),
        cmocka_unit_test(test_encode_read_response_success),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
