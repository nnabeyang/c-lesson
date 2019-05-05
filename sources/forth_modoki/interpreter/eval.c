#include "clesson.h"
#include<assert.h>
#include<stdio.h>
#include<string.h>

struct Stack* stack;
int streq(const char *s1, const char *s2) {
    return strcmp(s1, s2) == 0;
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
                        struct Token* right = stack_pop(stack);
                        struct Token* left = stack_pop(stack);
                        struct Token sum = {NUMBER, {0}};
                        sum.u.number = left->u.number + right->u.number;
                        stack_push(stack, &sum);
                    } else {
                        stack_push(stack, &token);
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

static void assert_token(struct Token* actual, struct Token* expect) {
    assert(actual->ltype == expect->ltype);
    switch(actual->ltype) {
        case NUMBER:
            assert(actual->u.number == expect->u.number);
            break;
        case EXECUTABLE_NAME:
        case LITERAL_NAME:
            assert(strcmp(actual->u.name, expect->u.name) == 0);
            break;
        case OPEN_CURLY:
        case CLOSE_CURLY:
        case END_OF_FILE:
            assert(actual->u.onechar == expect->u.onechar);
            break;
        default:
            break;
    }
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
    int n = sizeof(expects) / sizeof(struct Token);
    for(int i = 0; i < n; i++) {
        assert_token(stack_pop(stack), &expects[i]);
    }
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


int main() {
    stack = new_stack();
    test_eval_num_one();
    test_eval_num_two();
    test_eval_num_add();
    test_eval_stack_literal_name();

    return 0;
}
