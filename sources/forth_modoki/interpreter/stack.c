#include "clesson.h"
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#define STACK_SIZE 1024

void stack_push(struct Stack* stack, struct Token* token) {
    stack->tokens[stack->n++] = *token;
}

struct Token* stack_pop(struct Stack* stack) {
    if(stack->n == 0) return 0;
    return &stack->tokens[stack->n-- - 1];
}

struct Stack* new_stack() {
    struct Stack* stack = malloc(sizeof(struct Stack) + sizeof(struct Token) * STACK_SIZE);
    stack->n = 0;
    return stack;
}

static void setup_test(struct Stack* stack) {
    struct Token inputs[2] = {
        {NUMBER, {.number = 123}},
        {EXECUTABLE_NAME, {.name = "abc"}}
    };

    stack_push(stack, &inputs[0]);
    stack_push(stack, &inputs[1]);
}

static void test_copy_token() {
    struct Stack* stack = new_stack();
    struct Token expects[] = {
        {NUMBER, {.number = 123}},
        {EXECUTABLE_NAME, {.name = "abc"}}
    };

    setup_test(stack);
    assert_token(stack_pop(stack), &expects[1]);
    assert_token(stack_pop(stack), &expects[0]);
    assert(stack_pop(stack) == 0);
}

static void test_pop_stack_contains_two_tokens() {
    struct Stack* stack = new_stack();
    struct Token inputs[2] = {
        {NUMBER, {.number = 123}},
        {EXECUTABLE_NAME, {.name = "abc"}}
    };

    stack_push(stack, &inputs[0]);
    stack_push(stack, &inputs[1]);

    assert_token(stack_pop(stack), &inputs[1]);
    assert_token(stack_pop(stack), &inputs[0]);

    assert(stack_pop(stack) == 0);
}

static void test_pop_stack_contains_one_token() {
    struct Stack* stack = new_stack();
    struct Token input = {NUMBER, {.number = 123}};

    stack_push(stack, &input);
    assert_token(stack_pop(stack), &input);

    assert(0 == stack_pop(stack));
}

static void test_pop_empty() {
    struct Stack* stack = new_stack();
    assert(0 == stack_pop(stack));
}

void stack_unit_tests() {
    test_pop_empty();
    test_pop_stack_contains_one_token();
    test_pop_stack_contains_two_tokens();
    test_copy_token();
}

void stack_print_all(struct Stack* stack) {
    struct Token* token;

    while((token = stack_pop(stack)) != 0) {
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
    struct Stack* stack = new_stack();
    int n = sizeof(inputs)/ sizeof(struct Token);
    for(int i = 0; i < n; i++) {
        stack_push(stack, &inputs[i]);
    }
    stack_print_all(stack);
    return 0;
}
#endif