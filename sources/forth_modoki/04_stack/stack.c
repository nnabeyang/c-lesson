#include "stack.h"
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

void stack_push(struct Stack* stack, struct Token* token) {
    stack->tokens[stack->n++] = token;
}

struct Token* stack_pop(struct Stack* stack) {
    if(stack->n == 0) return 0;
    return stack->tokens[stack->n-- - 1];
}

static void test_pop_stack_contains_one_token() {
    struct Stack* stack = malloc(sizeof(struct Stack) + sizeof(struct Token*) * STACK_SIZE);
    stack->n = 0;
    int expect_val = 123;
    int expect_type = NUMBER;
    struct Token input = {NUMBER, {expect_val}};

    stack_push(stack, &input);
    struct Token* actual = stack_pop(stack);
    assert(actual->ltype == expect_type);
    assert(actual->u.number = expect_val);

    assert(0 == stack_pop(stack));
}

static void test_pop_empty() {
    struct Stack* stack = malloc(sizeof(struct Stack) + sizeof(struct Token*) * STACK_SIZE);
    stack->n = 0;
    assert(0 == stack_pop(stack));
}

static void unit_tests() {
    test_pop_empty();
    test_pop_stack_contains_one_token();
}

int main() {
    unit_tests();

    return 0;
}