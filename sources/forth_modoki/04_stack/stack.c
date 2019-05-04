#include "stack.h"
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

struct Token* stack_pop(struct Stack* stack) {
    if(stack->n == 0) return 0;
    return stack->tokens[stack->n-- - 1];
}

static void test_pop_empty() {
    struct Stack* stack = malloc(sizeof(struct Stack) + sizeof(struct Token*) * STACK_SIZE);
    stack->n = 0;
    assert(0 == stack_pop(stack));
}

static void unit_tests() {
    test_pop_empty();
}

int main() {
    unit_tests();

    return 0;
}