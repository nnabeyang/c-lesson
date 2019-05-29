#include "common.h"
#include <string.h>
#include <assert.h>
#include <stdint.h>
// (b, bl)
static int is_branch(int word) {
    return (((word >> 24) & 0xF) & 0B1010) == 0B1010;
}
// data processing
static int is_mov(int word) {
    return (word & 0xE1A00000) == 0xE1A00000;
}

static int is_mov_i(int word) {
    return (word >> 25 & 0B1) == 1;
}

static int is_ldr(int word) {
    return (word & 0xE5900000) == 0xE5900000;
}

static int is_byte(int word) {
    return (word >> 22 & 0B1) == 1;
}

static int is_add(int word) {
    return (word & 0xE2800000) == 0xE2800000;
}

static int is_cmp(int word) {
    return (word & 0xE3500000) == 0xE3500000;
}

static int is_str(int word) {
    return (word & 0xE5800000) == 0xE5800000;
}

static int is_and(int word) {
    return (word & 0xE2000000) == 0xE2000000;
}

static int is_sub(int word) {
    return (word & 0xE2400000) == 0xE2400000;
}
static int is_stmdb(int word) {
    return (word & 0xE92D0000) == 0xE92D0000;
}

static int is_ldmia(int word) {
    return (word & 0xE8BD0000) == 0xE8BD0000;
}

static int sprint_reg_list(char* reg_buf, int n, int regs) {
        int shft = 0;
        int m = n;
        while(shft < 16) {
            if(regs & (1 << shft)) {
                if(m != n) {
                    n += sprintf(&reg_buf[n], ", ");
                }
                n += sprintf(&reg_buf[n], "r%d", shft);
            }
            shft++;
        }
    return n;
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
        int rn = word >> 12 & 0xf;
        if(is_mov_i(word)) {
            sprintf(buf, "mov r%d, #0x%X\n", rn, (word & 0xfff));
        } else {
            int rm = word & 0B1111;
            sprintf(buf, "mov r%d, r%d\n", rn, rm);
        }
        cl_printf(buf);
        return 1;
    }
    if(is_branch(word)) {
        const char* base_cmd = (((word >> 24) & 0B1) == 0B1)? "bl" : "b";
        char cmd[16] = {0};
        int cond = (word >> 28) &0xF;
        if(cond == 0B0000) sprintf(cmd, "%seq", base_cmd);
        if(cond == 0B0001) sprintf(cmd , "%sne", base_cmd);
        if(cond == 0B1010) sprintf(cmd, "%sge", base_cmd);
        if(cond == 0B1110) sprintf(cmd, "%s", base_cmd);
        if(cmd[0] != '\0') {
            int offset = word & 0xffffff;
            if(offset <= 0x7fffff) {
                sprintf(buf, "%s [r15, #0x%X]\n", cmd, offset << 2);
            } else {
                sprintf(buf, "%s [r15, #-0x%X]\n", cmd, (0x1000000 - offset) << 2);
            }
            cl_printf(buf);
            return 1;
        }
    }
    if(is_ldr(word)) {
        const char* cmd = (is_byte(word)) ? "ldrb" : "ldr";

        int offset = word & 0xfff;
        int rn = word >> 16 & 0xf;
        int rd = word >> 12 & 0xf;

        if(offset == 0) {
            sprintf(buf, "%s r%d, [r%d]\n", cmd, rd, rn);
        } else {
            sprintf(buf, "%s r%d, [r%d, #0x%X]\n", cmd, rd, rn, offset);
        }
        cl_printf(buf);
        return 1;
    }
    if(is_str(word)) {
        int rn = word >> 16 & 0xf;
        int rd = word >> 12 & 0xf;
        sprintf(buf, "str r%d, [r%d]\n", rd, rn);
        cl_printf(buf);
        return 1;
    }
    if(is_add(word)) {
        int rn = word >> 16 & 0xf;
        int rd = word >> 12 & 0xf;
        int op2 = word & 0xfff;
        sprintf(buf, "add r%d, r%d, #0x%X\n", rn, rd, op2);
        cl_printf(buf);
        return 1;
    }
    if(is_cmp(word)) {
        int offset = word & 0xfff;
        int rn = word >> 16 & 0xf;
        sprintf(buf, "cmp r%d, #0x%X\n", rn, offset);
        cl_printf(buf);
        return 1;
    }
    if(is_sub(word)) {
        int rn = word >> 16 & 0xf;
        int rd = word >> 12 & 0xf;
        int op2 = word & 0xfff;
        sprintf(buf, "sub r%d, r%d, #0x%X\n", rd, rn, op2);
        cl_printf(buf);
        return 1;
    }
    if(is_and(word)) {
        int rn = word >> 16 & 0xf;
        int rd = word >> 12 & 0xf;
        int op2 = word & 0xfff;
        sprintf(buf, "and r%d, r%d, #0x%X\n", rd, rn, op2);
        cl_printf(buf);
        return 1;
    }
    if(is_stmdb(word) || is_ldmia(word)) {
        const char* cmd = NULL;
        if(is_stmdb(word)) {
            cmd = "stmdb";
        }
        if(is_ldmia(word)) {
            cmd = "ldmia";
        }
        int n = sprintf(buf, "%s r13!, {", cmd);
        n = sprint_reg_list(buf, n, word & 0xFFFF);
        sprintf(&buf[n], "}\n");
        cl_printf(buf);
        return 1;
    }
    dump_hex(word);
    return 0;
}

static void assert_str_eq(const char* expect, const char* actual) {
    assert(strcmp(expect, actual) == 0);
}
static void test_print_asm(int word, const char* expect, int is_asm) {
    cl_enable_buffer_mode();
    assert(print_asm(word) == is_asm);
    assert_str_eq(expect, cl_get_result(0));
    cl_clear_output();
}
void test_move1() {
   test_print_asm(0xE3A01068, "mov r1, #0x68\n", 1);
}

void test_move2() {
    test_print_asm(0xE3A01065, "mov r1, #0x65\n", 1);
}

void test_move3() {
    test_print_asm(0xE3A02008, "mov r2, #0x8\n", 1);
}

void test_mov_no_immediate() {
    test_print_asm(0xE1A03001, "mov r3, r1\n", 1);
}

void test_mov_no_immediat2() {
    test_print_asm(0xE1A0f00E, "mov r15, r14\n", 1);
}

void test_b_positive() {
    test_print_asm(0xEA000018, "b [r15, #0x60]\n", 1);
}

void test_b_negative() {
    test_print_asm(0xEAFFFFFE, "b [r15, #-0x8]\n", 1);
}

void test_bge() {
    test_print_asm(0xAA000001, "bge [r15, #0x4]\n", 1);
}

void test_ldr() {
    test_print_asm(0xE59F0038, "ldr r0, [r15, #0x38]\n", 1);
}

void test_ldr2() {
    test_print_asm(0xE59F1024, "ldr r1, [r15, #0x24]\n", 1);
}

void test_str() {
    test_print_asm(0xe5801000, "str r1, [r0]\n", 1);
}

void test_str2() {
    test_print_asm(0xE5812000, "str r2, [r1]\n", 1);  
}

void test_dump_hex() {
    test_print_asm(0x64646464, "64 64 64 64\n", 0);
}

void test_ldrb() {
    test_print_asm(0xe5d03000, "ldrb r3, [r0]\n", 1);
}

void test_add() {
    test_print_asm(0xE2800001, "add r0, r0, #0x1\n", 1);
}

void test_add2() {
    test_print_asm(0xE2855037, "add r5, r5, #0x37\n", 1);
}

void test_cmp() {
        test_print_asm(0xE3530000, "cmp r3, #0x0\n", 1);
}

void test_cmp2() {
    test_print_asm(0xE3520000, "cmp r2, #0x0\n", 1);
}

void test_bne() {
        test_print_asm(0x1AFFFFFA, "bne [r15, #-0x18]\n", 1);
}

void test_bne2() {
        test_print_asm(0x1AFFFFEF, "bne [r15, #-0x44]\n", 1);
}

void test_and() {
    test_print_asm(0xE203500F, "and r5, r3, #0xF\n", 1);
}

void test_bl() {
    test_print_asm(0xEB000005, "bl [r15, #0x14]\n", 1);
}

void test_stmdb() {
    test_print_asm(0xE92D0002, "stmdb r13!, {r1}\n", 1);
}

void test_stmdb2() {
    test_print_asm(0xE92D4008, "stmdb r13!, {r3, r14}\n", 1);
}

void test_ldmia() {
    test_print_asm(0xE8BD0002, "ldmia r13!, {r1}\n", 1);
}

void test_ldmia2() {
    test_print_asm(0xE8BD4008, "ldmia r13!, {r3, r14}\n", 1);
}

void test_sub() {
    test_print_asm(0xE2411004, "sub r1, r1, #0x4\n", 1);
}

void unit_tests() {
    test_move1();
    test_move2();
    test_b_positive();
    test_b_negative();
    test_ldr();
    test_str();
    test_dump_hex();
    test_ldr2();
    test_ldrb();
    test_add();
    test_cmp();
    test_bne();
    test_move3();
    test_mov_no_immediate();
    test_add2();
    test_and();
    test_cmp2();
    test_bne2();
    test_bge();
    test_mov_no_immediat2();
    test_bl();
    test_stmdb();
    test_stmdb2();
    test_ldmia();
    test_ldmia2();
    test_str2();
    test_sub();
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
