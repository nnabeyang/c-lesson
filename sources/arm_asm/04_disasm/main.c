#include "common.h"
#include <string.h>
#include <assert.h>
// (b, bl)
static int is_branch(int word) {
    return (word & 0xEA000000) == 0xEA000000;
}
// data processing
static int is_mov(int word) {
    return (word & 0xE3A01000) == 0xE3A01000;
}

int print_asm(int word) {
    if(is_mov(word)) {
        char buf[80];
        sprintf(buf, "mov r1, #0x%x\n", (word & 0xfff));
        cl_printf(buf);
        return 1;
    }
    if(is_branch(word)) {
        int offset = word & 0xffffff;
        char buf[80];
        if(offset <= 0x7fffff) {
            sprintf(buf, "b [r15, #0x%x]\n", offset);
        } else {
            sprintf(buf, "b [r15, #-0x%x]\n", (0x1000000 - offset));
        }
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

void test_b_positive() {
    cl_enable_buffer_mode();
    print_asm(0xea000060);
    char *actual = cl_get_result(0);
    assert_str_eq("b [r15, #0x60]\n", actual);
    cl_clear_output();
}

void test_b_negative() {
    cl_enable_buffer_mode();
    print_asm(0xeafffff8);
    char *actual = cl_get_result(0);
    assert_str_eq("b [r15, #-0x8]\n", actual);
    cl_clear_output();
}

void unit_tests() {
    test_move1();
    test_move2();
    test_b_positive();
    test_b_negative();
}

int main() {
    unit_tests();
    return 0;
}