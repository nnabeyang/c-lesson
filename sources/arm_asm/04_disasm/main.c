#include "common.h"
#include <string.h>
#include <assert.h>

int print_asm(int word) {
    if(((word >> 12) & 0xfffff) == 0xE3A01) {
        char buf[80];
        sprintf(buf, "mov r1, #0x%x\n", (word & 0xfff));
        cl_printf(buf);
        return 1;
    }
    return 0;
}

static void assert_str_eq(const char* expect, const char* actual) {
    assert(strcmp(expect, actual) == 0);
}

void test_move1() {
    cl_enable_buffer_mode();
    print_asm(0xE3A01068);
    char *actual = cl_get_result(0);
    assert_str_eq("mov r1, #0x68\n", actual);
    cl_clear_output();
}

void test_move2() {
    cl_enable_buffer_mode();
    print_asm(0xe3a01065);
    char *actual = cl_get_result(0);
    assert_str_eq("mov r1, #0x65\n", actual);
    cl_clear_output();
}

void unit_tests() {
    test_move1();
    test_move2();
}

int main() {
    unit_tests();
    return 0;
}