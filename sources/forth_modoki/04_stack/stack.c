#include "stack.h"
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

void stack_push(struct Stack* stack, struct Token* token) {
    stack->tokens[stack->n++] = token;
}

struct Token* stack_pop(struct Stack* stack) {
    if(stack->n == 0) return 0;
    return stack->tokens[stack->n-- - 1];
}

static struct Stack* newStack() {
    struct Stack* stack = malloc(sizeof(struct Stack) + sizeof(struct Token*) * STACK_SIZE);
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
}

int main() {
    unit_tests();

    return 0;
}