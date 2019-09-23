#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/mman.h>

#include "parser.h"
#include "test_util.h"

extern int eval(int r0, int r1, char *str);

/*
JIT
*/
int *binary_buf = NULL;

int* allocate_executable_buf(int size) {
    return (int*)mmap(0, size,
                 PROT_READ | PROT_WRITE | PROT_EXEC,
                 MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
}

void ensure_jit_buf() {
    if(binary_buf == NULL) {
        binary_buf = allocate_executable_buf(1024);
    }
}

static int pos = 0;

void push_number(int v) {
    binary_buf[pos++] = 0xe3a02000 + v;  // mov	r2, #v
    binary_buf[pos++] = 0xe92d0004;      // stmdb r13!, {r2}
}
void push_var(int reg) {
    binary_buf[pos++] = 0xe92d0000 + (1 << reg); //ldmia r13!, {r1}
}
void pop() {
    binary_buf[pos++] = 0xe8bd000c; //ldmia r13!, {r2, r3}
}
void add_and_push() {
    binary_buf[pos++] = 0xe0832002; //add	r2, r3, r2
    binary_buf[pos++] = 0xe92d0004;	//stmdb	r13!, {r2}
}
void sub_and_push() {
    binary_buf[pos++] = 0xe0432002; //sub	r2, r3, r2
    binary_buf[pos++] = 0xe92d0004; //stmdb	r13!, {r2}
}
void mul_and_push() {
    binary_buf[pos++] = 0xe0020293; //mul	r2, r3, r2
    binary_buf[pos++] = 0xe92d0004; //stmdb	r13!, {r2}
}
void div_and_push() {
    binary_buf[pos++] = 0xe732f213; //udiv	r2, r3, r2
    binary_buf[pos++] = 0xe92d0004; //stmdb	r13!, {r2}
}

int* jit_script(char *input) {
    ensure_jit_buf();
    struct Substr remain={input, strlen(input)};
    int val;

    pos = 0;

    while(!is_end(&remain)) {
        skip_space(&remain);
        if(is_number(remain.ptr)) {
            push_number(parse_number(remain.ptr));
            skip_token(&remain);
            continue;
        }else if(is_register(remain.ptr)) {
            if(remain.ptr[1] == '1') {
               push_var(1);
            } else {
              push_var(0);
            }
            skip_token(&remain);
            continue;
        } else {
            // must be op.
            val = parse_word(&remain);
            skip_token(&remain);
            pop();
            switch(val) {
                case OP_ADD:
                    add_and_push();
                    break;
                case OP_SUB:
                    sub_and_push();
                    break;
                case OP_MUL:
                    mul_and_push();
                    break;
                case OP_DIV:
                   div_and_push();
                    break;
            }
            continue;
        }
    }

    binary_buf[pos++] = 0xe8bd0001;  // ldmia	r13!, {r0}
    binary_buf[pos++] = 0xe1a0f00e;  // mov r15, r14
    return binary_buf;
}

void test_val() {
    int (*funcvar)(int, int);
    funcvar = (int(*)(int, int))jit_script("3");
    int res = funcvar(1, 1);
    assert_int_eq(3, res);

    funcvar = (int(*)(int, int))jit_script("5");
    res = funcvar(1, 1);
    assert_int_eq(5, res);
}

void test_add() {
    int (*funcvar)(int, int);
    funcvar = (int(*)(int, int))jit_script("3 7 add");
    int res = funcvar(1, 1);
    assert_int_eq(10, res);

    funcvar = (int(*)(int, int))jit_script("5 9 add");
    res = funcvar(1, 1);
    assert_int_eq(14, res);
}

void test_add_var() {
    int (*funcvar)(int, int);
    funcvar = (int(*)(int, int))jit_script("3 r1 add");

    assert_int_eq(4, funcvar(1, 1));
    assert_int_eq(8, funcvar(1, 5));

    funcvar = (int(*)(int, int))jit_script("3 r0 add");
    assert_int_eq(4, funcvar(1, 1));
    assert_int_eq(5, funcvar(2, 1));

    funcvar = (int(*)(int, int))jit_script("r0 r1 add");
    assert_int_eq(2, funcvar(1, 1));
    assert_int_eq(3, funcvar(2, 1));
}

void test_sub() {
    int (*funcvar)(int, int);
    funcvar = (int(*)(int, int))jit_script("7 3 sub");
    int res = funcvar(1, 1);
    assert_int_eq(4, res);

    funcvar = (int(*)(int, int))jit_script("15 9 sub");
    res = funcvar(1, 1);
    assert_int_eq(6, res);
}


void test_sub_var() {
    int (*funcvar)(int, int);
    funcvar = (int(*)(int, int))jit_script("3 r1 sub");

    assert_int_eq(2, funcvar(1, 1));
    assert_int_eq(-2, funcvar(1, 5));

    funcvar = (int(*)(int, int))jit_script("3 r0 sub");
    assert_int_eq(2, funcvar(1, 1));
    assert_int_eq(1, funcvar(2, 1));

    funcvar = (int(*)(int, int))jit_script("r0 r1 sub");
    assert_int_eq(0, funcvar(1, 1));
    assert_int_eq(1, funcvar(2, 1));
}

void test_mul() {
    int (*funcvar)(int, int);
    funcvar = (int(*)(int, int))jit_script("7 3 mul");
    int res = funcvar(1, 1);
    assert_int_eq(21, res);
    funcvar = (int(*)(int, int))jit_script("6 9 mul");
    res = funcvar(1, 1);
    assert_int_eq(54, res);
}

void test_mul_var() {
    int (*funcvar)(int, int);
    funcvar = (int(*)(int, int))jit_script("3 r1 mul");

    assert_int_eq(3, funcvar(1, 1));
    assert_int_eq(15, funcvar(1, 5));

    funcvar = (int(*)(int, int))jit_script("3 r0 mul");
    assert_int_eq(3, funcvar(1, 1));
    assert_int_eq(6, funcvar(2, 1));

    funcvar = (int(*)(int, int))jit_script("r0 r1 mul");
    assert_int_eq(15, funcvar(3, 5));
    assert_int_eq(63, funcvar(7, 9));
}

void test_div() {
    int (*funcvar)(int, int);
    funcvar = (int(*)(int, int))jit_script("10 2 div");
    int res = funcvar(1, 1);
    assert_int_eq(5, res);

    funcvar = (int(*)(int, int))jit_script("13 3 div");
    res = funcvar(1, 1);
    assert_int_eq(4, res);
}

void test_div_var() {
    int (*funcvar)(int, int);
    funcvar = (int(*)(int, int))jit_script("24 r1 div");

    assert_int_eq(3, funcvar(1, 8));
    assert_int_eq(12, funcvar(1, 2));

    funcvar = (int(*)(int, int))jit_script("38 r0 div");
    assert_int_eq(19, funcvar(2, 1));
    assert_int_eq(4, funcvar(8, 1));

    funcvar = (int(*)(int, int))jit_script("r0 r1 div");
    assert_int_eq(2, funcvar(12, 5));
    assert_int_eq(9, funcvar(81, 9));
}

static void run_unit_tests() {
    test_val();
    test_add();
    test_sub();
    test_mul();
    test_div();
    test_add_var();
    test_sub_var();
    test_mul_var();
    test_div_var();
    printf("all test done\n");
}


int main() {
    int res;
    int (*funcvar)(int, int);

    run_unit_tests();

    res = eval(1, 5, "3 7 add r1 sub 4 mul");
    printf("res=%d\n", res);

    /*
     TODO: Make below test pass.
    */
    funcvar = (int(*)(int, int))jit_script("3 7 add r1 sub 4 mul");

    res = funcvar(1, 5);
    assert_int_eq(20, res);

    res = funcvar(1, 4);
    assert_int_eq(24, res);

    return 0;
}

