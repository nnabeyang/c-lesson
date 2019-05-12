#include "clesson.h"
#include<assert.h>
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#define MAX_NAME_OP_NUMBERS 256

void add_op() {
    struct Token* right = stack_pop();
    struct Token* left = stack_pop();
    struct Token sum = {NUMBER, {.number = left->u.number + right->u.number}};
    stack_push(&sum);
}
void sub_op() {
    struct Token* right = stack_pop();
    struct Token* left = stack_pop();
    struct Token mul = {NUMBER, {.number = left->u.number - right->u.number}};
    stack_push(&mul);
}
void mul_op() {
    struct Token* right = stack_pop();
    struct Token* left = stack_pop();
    struct Token mul = {NUMBER, {.number = left->u.number * right->u.number}};
    stack_push(&mul);
}
void div_op() {
    struct Token* right = stack_pop();
    struct Token* left = stack_pop();
    struct Token mul = {NUMBER, {.number = left->u.number / right->u.number}};
    stack_push(&mul);
}
void def_op() {
    struct Token* value = stack_pop();
    struct Token* key = stack_pop();
    dict_put(key->u.name, value);
}

void register_primitives() {
    struct Token add = {ELEMENT_C_FUNC, {.cfunc = add_op}};
    dict_put("add", &add);
    struct Token sub = {ELEMENT_C_FUNC, {.cfunc = sub_op}};
    dict_put("sub", &sub);
    struct Token mul = {ELEMENT_C_FUNC, {.cfunc = mul_op}};
    dict_put("mul", &mul);
    struct Token div = {ELEMENT_C_FUNC, {.cfunc = div_op}};
    dict_put("div", &div);
    struct Token def = {ELEMENT_C_FUNC, {.cfunc = def_op}};
    dict_put("def", &def);
}

static int compile_exec_array(int ch, struct Token* out_token) {
    struct Token tokens[MAX_NAME_OP_NUMBERS];
    struct Token token = {
        OPEN_CURLY,
        {.onechar = '{'}
    };
    int len = 0;
    do {
        ch = parse_one(ch, &token);
        if(token.ltype != UNKNOWN) {
            switch(token.ltype) {
                case NUMBER:
                case EXECUTABLE_NAME:
                case LITERAL_NAME:
                    tokens[len++] = token;
                    break;
                case OPEN_CURLY:
                    ch = compile_exec_array(ch, &tokens[len++]);
                    ch = parse_one(ch, &token);
                    assert(token.ltype == CLOSE_CURLY);
                    break;
                case SPACE:
                    break;
                default:
                    printf("2:Unknown type %d\n", token.ltype);
                    break;
            }
        }
    }while(ch != '}');
    int size = sizeof(struct ElementArray) + sizeof(struct Token) * len;
    struct ElementArray* byte_codes = malloc(size);
    byte_codes->len = len;
    memcpy(byte_codes->elements, tokens, size);
    struct Token t = {EXECUTABLE_ARRAY, {.byte_codes = byte_codes}};
    *out_token = t;
    return ch;
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
                    stack_push(&token);
                    break;
                case SPACE:
                    break;
                case OPEN_CURLY: {
                    struct Token out_token;
                    ch = compile_exec_array(ch, &out_token);
                    stack_push(&out_token);
                }
                break;
                case CLOSE_CURLY:
                break;
                case EXECUTABLE_NAME: {
                        struct Token elem;
                        if(dict_get(token.u.name, &elem) && elem.ltype == ELEMENT_C_FUNC) {
                            elem.u.cfunc();
                        } else {
                            struct Token out_val;
                            if(dict_get(token.u.name, &out_val)) {
                                stack_push(&out_val);
                            } else {
                                stack_push(&token);
                            }
                        }
                    }
                    break;
                case LITERAL_NAME:
                    stack_push(&token);
                    break;
                default:
                    printf("Unknown type %d\n", token.ltype);
                    break;
            }
        }
    }while(ch != EOF);
}

static void test_eval_exec_array_nest() {
    char *input = "{1 {2} 3}";
    struct Token expects[] = {
        {NUMBER, {.number = 1}},
        {NUMBER, {.number = 2}},
        {NUMBER, {.number = 3}}
    };
    cl_getc_set_src(input);
    eval();
    struct Token* token = stack_pop();
    assert(token->ltype == EXECUTABLE_ARRAY);
    struct ElementArray* byte_codes = token->u.byte_codes;
    assert(byte_codes->len == 3);

    struct Token element = byte_codes->elements[0];
    assert_token(&element, &expects[0]);

    element = byte_codes->elements[1];
    assert(element.ltype == EXECUTABLE_ARRAY);
    assert(element.u.byte_codes->len == 1);
    element = element.u.byte_codes->elements[0];
    assert_token(&element, &expects[1]);

    element = byte_codes->elements[2];
    assert_token(&element, &expects[2]);

    assert(stack_pop() == 0);
}

static void test_eval_exec_array_two_element_array() {
    char *input = "{1} {2}";
    struct Token expects[] = {
        {NUMBER, {.number = 2}},
        {NUMBER, {.number = 1}}
    };
    cl_getc_set_src(input);
    eval();
    {
    struct Token* token = stack_pop();
    assert(token->ltype == EXECUTABLE_ARRAY);
    struct ElementArray* byte_codes = token->u.byte_codes;
    assert(byte_codes->len == 1);
    struct Token element = byte_codes->elements[0];
    assert_token(&element, &expects[0]);
    }
    {
    struct Token* token = stack_pop();
    assert(token->ltype == EXECUTABLE_ARRAY);
    struct ElementArray* byte_codes = token->u.byte_codes;
    assert(byte_codes->len == 1);
    struct Token element = byte_codes->elements[0];
    assert_token(&element, &expects[1]);
    }
    assert(stack_pop() == 0);

}

static void test_eval_exec_array_two_element() {
    char *input = "{1 2}";
    struct Token expects[] = {
        {NUMBER, {.number = 1}},
        {NUMBER, {.number = 2}}
    };
    cl_getc_set_src(input);
    eval();
    struct Token* token = stack_pop();
    assert(token->ltype == EXECUTABLE_ARRAY);
    struct ElementArray* byte_codes = token->u.byte_codes;
    assert(byte_codes->len == 2);
    struct Token element = byte_codes->elements[0];
    assert_token(&element, &expects[0]);
    element = byte_codes->elements[1];
    assert_token(&element, &expects[1]);
    assert(stack_pop() == 0);
}

static void test_eval_exec_array(char *input, struct Token* expect) {
    cl_getc_set_src(input);
    eval();
    struct Token* token = stack_pop();
    assert(token->ltype == EXECUTABLE_ARRAY);
    struct ElementArray* byte_codes = token->u.byte_codes;
    assert(byte_codes->len == 1);
    struct Token element = byte_codes->elements[0];
    assert_token(&element, expect);
    assert(stack_pop() == 0);
}

static void test_eval_exec_array_one() {
    char *inputs[] = {
        "{27}",
        "{/abc}",
        "{abc}"
    };
    struct Token expects[] = {
        {NUMBER, {.number= 27}},
        {LITERAL_NAME, {.name = "abc"}},
        {EXECUTABLE_NAME, {.name = "abc"}}
    };
    int n = sizeof(inputs)/ sizeof(char*);
    for(int i = 0; i < n; i++) {
        test_eval_exec_array(inputs[i], &expects[i]);
    }
}

static void test_eval_divide() {
    char *input = "15 3 div";
    struct Token expect = {NUMBER, {.number= 5}};

    cl_getc_set_src(input);

    eval();
    assert_token(stack_pop(), &expect);
    assert(stack_pop() == 0);
}

static void test_eval_subtract() {
    char *input = "15 7 sub";
    struct Token expect = {NUMBER, {.number= 8}};

    cl_getc_set_src(input);

    eval();
    assert_token(stack_pop(), &expect);
    assert(stack_pop() == 0);
}

static void test_eval_multiply() {
    char *input = "2 3 mul";
    struct Token expect = {NUMBER, {.number= 6}};

    cl_getc_set_src(input);

    eval();
    assert_token(stack_pop(), &expect);
    assert(stack_pop() == 0);
}

static void test_eval_no_expression() {
    char *input = "/abc 12 def\n"
                  "abc abc";
    struct Token expect = {NUMBER, {.number= 12}};

    cl_getc_set_src(input);

    eval();
    assert_token(stack_pop(), &expect);
    assert_token(stack_pop(), &expect);
    assert(stack_pop() == 0);
}

static void test_eval_num_add3() {
    char *input = "/abc 12 def\n"
                  "abc abc add";
    int expect = 24;

    cl_getc_set_src(input);

    eval();

    int actual = stack_pop()->u.number;
    assert(expect == actual);
}

static void test_eval_num_add2() {
    char *input = "/hoge 123 def hoge 2 add";
    int expect = 125;

    cl_getc_set_src(input);

    eval();

    int actual = stack_pop()->u.number;
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
    assert(stack_pop() == 0);
}

static void test_eval_num_one() {
    char *input = "123";
    int expect = 123;

    cl_getc_set_src(input);
    eval();

    int actual = stack_pop()->u.number;
    assert(expect == actual);
}

static void test_eval_num_two() {
    char *input = "123 456";
    int expect1 = 456;
    int expect2 = 123;

    cl_getc_set_src(input);

    eval();

    int actual1 = stack_pop()->u.number;
    int actual2 = stack_pop()->u.number;

    assert(expect1 == actual1);
    assert(expect2 == actual2);
}


static void test_eval_num_add() {
    char *input = "1 2 add";
    int expect = 3;

    cl_getc_set_src(input);

    eval();

    assert(stack_pop()->u.number == expect);
}

static void eval_unit_tests() {
    void (* tests[])(void) = {
        test_eval_num_one,
        test_eval_num_two,
        test_eval_num_add,
        test_eval_stack_literal_name,
        test_eval_num_add2,
        test_eval_num_add3,
        test_eval_no_expression,
        test_eval_multiply,
        test_eval_subtract,
        test_eval_divide,
        test_eval_exec_array_one,
        test_eval_exec_array_two_element,
        test_eval_exec_array_two_element_array,
        test_eval_exec_array_nest
    };
    int n = sizeof(tests)/ sizeof(void (*)());
    for(int i = 0; i < n; i++) {
        do_test(tests[i]);
    }
}

static void unit_tests() {
    stack_unit_tests();
    dict_unit_tests();
    register_primitives();
    eval_unit_tests();
}

int main() {
    unit_tests();

    return 0;
}
