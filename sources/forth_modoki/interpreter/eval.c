#include "clesson.h"
#include<assert.h>
#include<stdio.h>
#include<string.h>

struct Stack* stack;
void add_op() {
    struct Token* right = stack_pop(stack);
    struct Token* left = stack_pop(stack);
    struct Token sum = {NUMBER, {.number = left->u.number + right->u.number}};
    stack_push(stack, &sum);
}

void eval() {
    int ch = EOF;
    struct Token token = {
        UNKNOWN,
        {0}
    };
    do {
        ch = parse_one(ch, &token);
        if(token.ltype != UNKNOWN) {
            switch(token.ltype) {
                case NUMBER:
                    stack_push(stack, &token);
                    break;
                case SPACE:
                    break;
                case EXECUTABLE_NAME:
                    if(streq(token.u.name, "add")) {
                        add_op();
                    } else if (streq(token.u.name, "def")) {
                        struct Token* value = stack_pop(stack);
                        struct Token* key = stack_pop(stack);
                        dict_put(key->u.name, value);
                    } else {
                        struct Token out_val;
                        if(dict_get(token.u.name, &out_val)) {
                            stack_push(stack, &out_val);
                        } else {
                            stack_push(stack, &token);
                        }
                    }
                    break;
                case LITERAL_NAME:
                    stack_push(stack, &token);
                    break;
                default:
                    printf("Unknown type %d\n", token.ltype);
                    break;
            }
        }
    }while(ch != EOF);
}

static void test_eval_no_expression() {
    char *input = "/abc 12 def\n"
                  "abc abc";
    struct Token expect = {NUMBER, {.number= 12}};

    cl_getc_set_src(input);

    eval();
    assert_token(stack_pop(stack), &expect);
    assert_token(stack_pop(stack), &expect);
    assert(stack_pop(stack) == 0);
}

static void test_eval_num_add3() {
    char *input = "/abc 12 def\n"
                  "abc abc add";
    int expect = 24;

    cl_getc_set_src(input);

    eval();

    int actual = stack_pop(stack)->u.number;
    assert(expect == actual);
}

static void test_eval_num_add2() {
    char *input = "/hoge 123 def hoge 2 add";
    int expect = 125;

    cl_getc_set_src(input);

    eval();

    int actual = stack_pop(stack)->u.number;
    assert(expect == actual);
}

static void test_eval_stack_literal_name() {
    char *input = "/hoge 123 def";
    struct Token expects[] = {
        {EXECUTABLE_NAME, {.name = "def"}},
        {NUMBER, {.number = 123}},
        {LITERAL_NAME, {.name = "hoge"}}
    };

    cl_getc_set_src(input);
    eval();
    assert(stack_pop(stack) == 0);
}

static void test_eval_num_one() {
    char *input = "123";
    int expect = 123;

    cl_getc_set_src(input);
    eval();

    int actual = stack_pop(stack)->u.number;
    assert(expect == actual);
}

static void test_eval_num_two() {
    char *input = "123 456";
    int expect1 = 456;
    int expect2 = 123;

    cl_getc_set_src(input);

    eval();

    int actual1 = stack_pop(stack)->u.number;
    int actual2 = stack_pop(stack)->u.number;

    assert(expect1 == actual1);
    assert(expect2 == actual2);
}


static void test_eval_num_add() {
    char *input = "1 2 add";
    int expect = 3;

    cl_getc_set_src(input);

    eval();

    int actual = stack_pop(stack)->u.number;
    assert(expect == actual);
}

static void eval_unit_tests() {
    stack = new_stack();
    test_eval_num_one();
    test_eval_num_two();
    test_eval_num_add();
    test_eval_stack_literal_name();
    test_eval_num_add2();
    test_eval_num_add3();
    test_eval_no_expression();
}

static void unit_tests() {
    stack_unit_tests();
    dict_unit_tests();
    eval_unit_tests();
}

int main() {
    unit_tests();

    return 0;
}
