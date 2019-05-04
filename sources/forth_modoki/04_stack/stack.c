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

static void test_pop_stack_contains_two_tokens() {
    struct Stack* stack = newStack();
    int expect_val0 = 123;
    const char* expect_val1 = "add";
    int expect_types[2] = {NUMBER, EXECUTABLE_NAME};
    struct Token inputs[2] = {
        {NUMBER, {0}},
        {EXECUTABLE_NAME, {0}}
    };
    inputs[0].u.number = 123;
    inputs[1].u.name = "abc";

    stack_push(stack, &inputs[0]);
    stack_push(stack, &inputs[1]);
    struct Token* actual;

    actual = stack_pop(stack);
    assert(actual->ltype == expect_types[1]);
    assert(strcmp(actual->u.name, expect_val1));

    actual = stack_pop(stack);
    assert(actual->ltype == expect_types[0]);
    assert(actual->u.number == expect_val0);

    assert(stack_pop(stack) == 0);
}

static void test_pop_stack_contains_one_token() {
    struct Stack* stack = newStack();
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