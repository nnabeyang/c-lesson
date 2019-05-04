#include "stack.h"
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

void stack_push(struct Stack* stack, struct Token* token) {
    stack->tokens[stack->n++] = *token;
}

struct Token* stack_pop(struct Stack* stack) {
    if(stack->n == 0) return 0;
    return &stack->tokens[stack->n-- - 1];
}

static struct Stack* newStack() {
    struct Stack* stack = malloc(sizeof(struct Stack) + sizeof(struct Token) * STACK_SIZE);
    stack->n = 0;
    return stack;
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
static void setup_test(struct Stack* stack) {
    struct Token inputs[2] = {
        {NUMBER, {0}},
        {EXECUTABLE_NAME, {0}}
    };
    inputs[0].u.number = 123;
    inputs[1].u.name = "abc";

    stack_push(stack, &inputs[0]);
    stack_push(stack, &inputs[1]);
}

static void test_copy_token() {
    struct Stack* stack = newStack();
    struct Token expects[] = {
        {NUMBER, {0}},
        {EXECUTABLE_NAME, {0}}
    };
    expects[0].u.number = 123;
    expects[1].u.name = "abc";
    setup_test(stack);
    assert_token(stack_pop(stack), &expects[1]);
    assert_token(stack_pop(stack), &expects[0]);
    assert(stack_pop(stack) == 0);
}

static void test_pop_stack_contains_two_tokens() {
    struct Stack* stack = newStack();
    struct Token inputs[2] = {
        {NUMBER, {0}},
        {EXECUTABLE_NAME, {0}}
    };
    inputs[0].u.number = 123;
    inputs[1].u.name = "abc";

    stack_push(stack, &inputs[0]);
    stack_push(stack, &inputs[1]);

    assert_token(stack_pop(stack), &inputs[1]);
    assert_token(stack_pop(stack), &inputs[0]);

    assert(stack_pop(stack) == 0);
}

static void test_pop_stack_contains_one_token() {
    struct Stack* stack = newStack();
    struct Token input = {NUMBER, {0}};
    input.u.number = 123;

    stack_push(stack, &input);
    assert_token(stack_pop(stack), &input);

    assert(0 == stack_pop(stack));
}

static void test_pop_empty() {
    struct Stack* stack = newStack();
    assert(0 == stack_pop(stack));
}

static void unit_tests() {
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

int main() {
    unit_tests();
    struct Token inputs[] = {
        {NUMBER, {0}},
        {EXECUTABLE_NAME, {0}},
        {SPACE, {0}},
        {OPEN_CURLY, {0}},
        {CLOSE_CURLY, {0}},
        {LITERAL_NAME, {0}},
        {UNKNOWN, {0}}
    };
    inputs[0].u.number = 123;
    inputs[1].u.name = "abc";
    inputs[2].u.onechar = ' ';
    inputs[3].u.onechar = '{';
    inputs[4].u.onechar = '}';
    inputs[5].u.name = "def";
    struct Stack* stack = newStack();
    int n = sizeof(inputs)/ sizeof(struct Token);
    for(int i = 0; i < n; i++) {
        stack_push(stack, &inputs[i]);
    }
    stack_print_all(stack);
    return 0;
}