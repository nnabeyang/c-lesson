#include "common.h"
#include <string.h>
#include <assert.h>
#include <stdint.h>
// (b, bl)
static int is_branch(int word) {
    return (word & 0xEA000000) == 0xEA000000;
}
// data processing
static int is_mov(int word) {
    return (word & 0xE3A01000) == 0xE3A01000;
}

static int is_ldr(int word) {
    return (word & 0xE5900000) == 0xE5900000;
}

static int is_str(int word) {
    return (word & 0xE5800000) == 0xE5800000;
}

void dump_hex(int word) {
    int n = 24;
    int vs[4];
    for(int i = 0; i < 4; i++) {
        vs[i] = (word >> n) & 0xFF;
        n -= 8;
    }
    char buf[80];
    sprintf(buf, "%02X %02X %02X %02X\n", vs[0], vs[1], vs[2], vs[3]);
    cl_printf(buf);
}

int print_asm(int word) {
    char buf[80];
    if(is_mov(word)) {
        sprintf(buf, "mov r1, #0x%x\n", (word & 0xfff));
        cl_printf(buf);
        return 1;
    }
    if(is_branch(word)) {
        int offset = word & 0xffffff;
        if(offset <= 0x7fffff) {
            sprintf(buf, "b [r15, #0x%x]\n", offset << 2);
        } else {
            sprintf(buf, "b [r15, #-0x%x]\n", (0x1000000 - offset) << 2);
        }
        cl_printf(buf);
        return 1;
    }
    if(is_ldr(word)) {
        sprintf(buf, "ldr r0, [r15, #0x38]\n");
        cl_printf(buf);
        return 1;
    }
    if(is_str(word)) {
        sprintf(buf, "str r1, [r0]\n");
        cl_printf(buf);
        return 1;
    }
    dump_hex(word);
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
    print_asm(0xea000018);
    char *actual = cl_get_result(0);
    assert_str_eq("b [r15, #0x60]\n", actual);
    cl_clear_output();
}

void test_b_negative() {
    cl_enable_buffer_mode();
    print_asm(0xeafffffe);
    char *actual = cl_get_result(0);
    assert_str_eq("b [r15, #-0x8]\n", actual);
    cl_clear_output();
}

void test_ldr() {
    cl_enable_buffer_mode();
    print_asm(0xe59f0038);
    char *actual = cl_get_result(0);
    assert_str_eq("ldr r0, [r15, #0x38]\n", actual);
    cl_clear_output();   
}

void test_str() {
    cl_enable_buffer_mode();
    print_asm(0xe5801000);
    char *actual = cl_get_result(0);
    assert_str_eq("str r1, [r0]\n", actual);
    cl_clear_output();
}

void unit_tests() {
    test_move1();
    test_move2();
    test_b_positive();
    test_b_negative();
    test_ldr();
    test_str();
}

int main(int argc, char *argv[]) {
    if(argc == 1) {
        unit_tests();
    } else if(argc == 2) {
        uint32_t word;
        unsigned int size_word = sizeof(word);
        FILE* fp = NULL;
        fp = fopen(argv[1], "rb");
        if(fp == NULL) {
            fputs("ファイルオープンに失敗しました。\n", stderr);
            return 1;
        }
        uint32_t addr = 0x00010000;
        while(fread(&word, size_word, 1, fp) == 1) {
                cl_enable_buffer_mode();
                print_asm(word);
                printf("0x%X %s", addr, cl_get_result(0));
                addr += 4;
                cl_clear_output();
        }
        fclose(fp);
        return 0;
    }
    return 1;
}
