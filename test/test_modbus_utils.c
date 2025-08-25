#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdint.h>
#include <cmocka.h>

#include "modbus_utils.h"

static void test_is_valid_quantity(void **state) {
    (void) state;
    assert_true(is_valid_quantity(1));
    assert_true(is_valid_quantity(MODBUS_MAX_REGS));
    assert_false(is_valid_quantity(0));
    assert_false(is_valid_quantity(MODBUS_MAX_REGS + 1));
}

static void test_is_valid_byte_count(void **state) {
    (void) state;
    assert_true(is_valid_byte_count(2));
    assert_true(is_valid_byte_count(MODBUS_MAX_REGS * 2));
    assert_false(is_valid_byte_count(1));
    assert_false(is_valid_byte_count(MODBUS_MAX_REGS * 2 + 1));
}

static void test_is_valid_slave_id(void **state) {
    (void) state;
    assert_true(is_valid_slave_id(0));
    assert_true(is_valid_slave_id(MODBUS_MAX_SLAVES));
    assert_false(is_valid_slave_id(MODBUS_MAX_SLAVES + 1));
}

static void test_is_valid_address_range(void **state) {
    (void) state;
    assert_true(is_valid_address_range(0, 1));
    assert_true(is_valid_address_range(0xFFFE, 2));
    assert_false(is_valid_address_range(0xFFFF, 2));
    assert_false(is_valid_address_range(0xFFFE, 3));
}

static void test_modbus_crc16(void **state) {
    (void) state;
    const uint8_t data1[] = {0x01, 0x03, 0x00, 0x01, 0x00, 0x01};
    uint16_t crc = modbus_crc16(data1, sizeof(data1));
    assert_int_equal(crc, 0x8409); // Example expected CRC (verify with reference)

    const uint8_t empty[] = {};
    assert_int_equal(modbus_crc16(empty, 0), 0xFFFF);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_is_valid_quantity),
        cmocka_unit_test(test_is_valid_byte_count),
        cmocka_unit_test(test_is_valid_slave_id),
        cmocka_unit_test(test_is_valid_address_range),
        cmocka_unit_test(test_modbus_crc16),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
