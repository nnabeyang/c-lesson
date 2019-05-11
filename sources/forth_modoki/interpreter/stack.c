#include "clesson.h"
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#define STACK_SIZE 1024
static int stack_pos = 0;
static struct Token tokens[STACK_SIZE];

void stack_push(struct Token* token) {
    tokens[stack_pos++] = *token;
}

struct Token* stack_pop() {
    if(stack_pos == 0) return 0;
    return &tokens[stack_pos-- - 1];
}

void stack_reset() {
    stack_pos = 0;
}

static void setup_test() {
    struct Token inputs[2] = {
        {NUMBER, {.number = 123}},
        {EXECUTABLE_NAME, {.name = "abc"}}
    };

    stack_push(&inputs[0]);
    stack_push(&inputs[1]);
}

static void test_copy_token() {
    struct Token expects[] = {
        {NUMBER, {.number = 123}},
        {EXECUTABLE_NAME, {.name = "abc"}}
    };

    setup_test();
    assert_token(stack_pop(), &expects[1]);
    assert_token(stack_pop(), &expects[0]);
    assert(stack_pop() == 0);
}

static void test_pop_stack_contains_two_tokens() {
    struct Token inputs[2] = {
        {NUMBER, {.number = 123}},
        {EXECUTABLE_NAME, {.name = "abc"}}
    };

    stack_push(&inputs[0]);
    stack_push(&inputs[1]);

    assert_token(stack_pop(), &inputs[1]);
    assert_token(stack_pop(), &inputs[0]);

    assert(stack_pop() == 0);
}

static void test_pop_stack_contains_one_token() {
    struct Token input = {NUMBER, {.number = 123}};

    stack_push(&input);
    assert_token(stack_pop(), &input);

    assert(0 == stack_pop());
}

static void test_pop_empty() {
    stack_reset();
    assert(0 == stack_pop());
}

void stack_unit_tests() {
    test_pop_empty();
    test_pop_stack_contains_one_token();
    test_pop_stack_contains_two_tokens();
    test_copy_token();
}

void stack_print_all() {
    struct Token* token;

    while((token = stack_pop()) != 0) {
        if(token->ltype != UNKNOWN) {
            switch(token->ltype) {
                case NUMBER:
                    printf("num: %d\n", token->u.number);
                    break;
                case SPACE:
                    printf("space!\n");
                    break;
                case OPEN_CURLY:
                    printf("Open curly brace '%c'\n", token->u.onechar);
                    break;
                case CLOSE_CURLY:
                    printf("Close curly brace '%c'\n", token->u.onechar);
                    break;
                case EXECUTABLE_NAME:
                    printf("EXECUTABLE_NAME: %s\n", token->u.name);
                    break;
                case LITERAL_NAME:
                    printf("LITERAL_NAME: %s\n", token->u.name);
                    break;
                default:
                    printf("Unknown type %d\n", token->ltype);
                    break;
            }
        }
    }
}

#if 0
int main() {
    stack_unit_tests();
    struct Token inputs[] = {
        {NUMBER, {.number = 123}},
        {EXECUTABLE_NAME, {.name = "abc"}},
        {SPACE, {.onechar = ' '}},
        {OPEN_CURLY, {.onechar = '{'}},
        {CLOSE_CURLY, {.onechar = '}'}},
        {LITERAL_NAME, {.name = "def"}},
        {UNKNOWN, {0}}
    };
    inputs[0].u.number = 123;
    inputs[1].u.name = "abc";
    inputs[2].u.onechar = ' ';
    inputs[3].u.onechar = '{';
    inputs[4].u.onechar = '}';
    inputs[5].u.name = "def";

    int n = sizeof(inputs)/ sizeof(struct Token);
    for(int i = 0; i < n; i++) {
        stack_push(&inputs[i]);
    }
    stack_print_all();
    return 0;
}
#endif