#include "clesson.h"
#include<assert.h>
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#define MAX_NAME_OP_NUMBERS 256
void eval_exec_array();
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

void eq_op() {
    struct Token* right = stack_pop();
    struct Token* left = stack_pop();
    struct Token op = {NUMBER, {.number = left->u.number == right->u.number}};
    stack_push(&op);
}
void neq_op() {
    struct Token* right = stack_pop();
    struct Token* left = stack_pop();
    struct Token op = {NUMBER, {.number = left->u.number != right->u.number}};
    stack_push(&op);
}
void gt_op() {
    struct Token* right = stack_pop();
    struct Token* left = stack_pop();
    struct Token op = {NUMBER, {.number = left->u.number > right->u.number}};
    stack_push(&op);
}
void ge_op() {
    struct Token* right = stack_pop();
    struct Token* left = stack_pop();
    struct Token op = {NUMBER, {.number = left->u.number >= right->u.number}};
    stack_push(&op);
}
void lt_op() {
    struct Token* right = stack_pop();
    struct Token* left = stack_pop();
    struct Token op = {NUMBER, {.number = left->u.number < right->u.number}};
    stack_push(&op);
}
void le_op() {
    struct Token* right = stack_pop();
    struct Token* left = stack_pop();
    struct Token op = {NUMBER, {.number = left->u.number <= right->u.number}};
    stack_push(&op);
}

void pop_op() {
    stack_pop();
}
void exch_op() {
    struct Token right = *stack_pop();
    struct Token left = *stack_pop();
    stack_push(&right);
    stack_push(&left);
}
void dup_op() {
    struct Token* value = stack_pop();
    struct Token cpy = *value;
    stack_push(value);
    stack_push(&cpy);
}
void index_op() {
    struct Token* index = stack_pop();
    assert(index->ltype == NUMBER);
    int n = index->u.number;
    struct Token** tokens = malloc(sizeof(struct Token*) * n);
    struct Token** q = tokens;
    for(int i = 0; i <= n; i++) {
        *q++ = stack_pop();
    }
    struct Token value = *tokens[n];
    while(q-- != tokens) {
        stack_push(*q);
    }
    stack_push(&value);
}
void roll_op() {
    struct Token* j_token = stack_pop();
    assert(j_token->ltype == NUMBER);
    int j = j_token->u.number;
    struct Token* n_token = stack_pop();
    assert(n_token->ltype == NUMBER);
    int n = n_token->u.number;
    struct Token* tokens = malloc(sizeof(struct Token) * n);
    for(int i = 0; i < n; i++) {
        tokens[n - 1 - i] = *stack_pop();
    }
    for(int i = 0; i < n; i++) {
        stack_push(&tokens[(n + i - j) % n]);
    }
}

void exec_op() {
    struct Token* proc = stack_pop();
    assert(proc->ltype == EXECUTABLE_ARRAY);
    eval_exec_array(proc->u.byte_codes);
}
void if_op() {
    struct Token* proc_token = stack_pop();
    assert(proc_token->ltype == EXECUTABLE_ARRAY);
    struct ElementArray* proc = proc_token->u.byte_codes;
    struct Token* cond_token = stack_pop();
    assert(cond_token->ltype == NUMBER);
    int cond = cond_token->u.number;
    if(cond) {
        eval_exec_array(proc);
    }
}
void ifelse_op() {
    struct Token* else_token = stack_pop();
    assert(else_token->ltype == EXECUTABLE_ARRAY);
    struct ElementArray* else_proc = else_token->u.byte_codes;

    struct Token* then_token = stack_pop();
    assert(then_token->ltype == EXECUTABLE_ARRAY);
    struct ElementArray* then_proc = then_token->u.byte_codes;

    struct Token* cond_token = stack_pop();
    assert(cond_token->ltype == NUMBER);
    int cond = cond_token->u.number;
    eval_exec_array((cond)? then_proc: else_proc);
}
void repeat_op() {
    struct Token* proc_token = stack_pop();
    assert(proc_token->ltype == EXECUTABLE_ARRAY);
    struct ElementArray* proc = proc_token->u.byte_codes;
    struct Token* n_token = stack_pop();
    assert(n_token->ltype == NUMBER);
    int n = n_token->u.number;
    for(int i = 0; i < n; i++) {
        eval_exec_array(proc);
    }
}

void while_op() {
    struct Token* body_token = stack_pop();
    assert(body_token->ltype == EXECUTABLE_ARRAY);
    struct ElementArray* body = body_token->u.byte_codes;
    struct Token* cond_token = stack_pop();
    assert(cond_token->ltype == EXECUTABLE_ARRAY);
    struct ElementArray* cond = cond_token->u.byte_codes;
    eval_exec_array(cond);
    int val = stack_pop()->u.number;
    while(val) {
        eval_exec_array(body);
        eval_exec_array(cond);
        val = stack_pop()->u.number;
    }
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

    {
    struct Token op = {ELEMENT_C_FUNC, {.cfunc = eq_op}};
    dict_put("eq", &op);
    }
    {
    struct Token op  = {ELEMENT_C_FUNC, {.cfunc = neq_op}};
    dict_put("neq", &op);
    }
    {
    struct Token op = {ELEMENT_C_FUNC, {.cfunc = gt_op}};
    dict_put("gt", &op);
    }
    {
    struct Token op  = {ELEMENT_C_FUNC, {.cfunc = ge_op}};
    dict_put("ge", &op);
    }
    {
    struct Token op = {ELEMENT_C_FUNC, {.cfunc = lt_op}};
    dict_put("lt", &op);
    }
    {
    struct Token op  = {ELEMENT_C_FUNC, {.cfunc = le_op}};
    dict_put("le", &op);
    }

    {
    struct Token op  = {ELEMENT_C_FUNC, {.cfunc = pop_op}};
    dict_put("pop", &op);
    }
    {
    struct Token op  = {ELEMENT_C_FUNC, {.cfunc = exch_op}};
    dict_put("exch", &op);
    }
    {
    struct Token op  = {ELEMENT_C_FUNC, {.cfunc = dup_op}};
    dict_put("dup", &op);
    }
    {
    struct Token op  = {ELEMENT_C_FUNC, {.cfunc = index_op}};
    dict_put("index", &op);
    }
    {
    struct Token op  = {ELEMENT_C_FUNC, {.cfunc = roll_op}};
    dict_put("roll", &op);
    }
    {
    struct Token op = {ELEMENT_C_FUNC, {.cfunc = exec_op}};
    dict_put("exec", &op);
    }
    {
    struct Token op = {ELEMENT_C_FUNC, {.cfunc = if_op}};
    dict_put("if", &op);
    }
    {
    struct Token op = {ELEMENT_C_FUNC, {.cfunc = ifelse_op}};
    dict_put("ifelse", &op);
    }
    {
    struct Token op = {ELEMENT_C_FUNC, {.cfunc = repeat_op}};
    dict_put("repeat", &op);
    }
    {
    struct Token op = {ELEMENT_C_FUNC, {.cfunc = while_op}};
    dict_put("while", &op);
    }
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
                        if(dict_get(token.u.name, &elem)) {
                            switch(elem.ltype) {
                            case ELEMENT_C_FUNC:
                                elem.u.cfunc();
                                break;
                            case EXECUTABLE_ARRAY:
                                eval_exec_array(elem.u.byte_codes);
                                break;
                            default: {
                                struct Token out_val;
                                if(dict_get(token.u.name, &out_val)) {
                                    stack_push(&out_val);
                                } else {
                                    stack_push(&token);
                                }
                            }
                            break;
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

void eval_exec_array(struct ElementArray* elems) {
    int n = elems->len;
    for(int i = 0; i < n; i++) {
        struct Token token = elems->elements[i];
        switch(token.ltype) {
        case NUMBER:
            stack_push(&token);
            break;
        case EXECUTABLE_ARRAY:
            stack_push(&token);
            break;
        case EXECUTABLE_NAME: {
            struct Token elem;
            if(dict_get(token.u.name, &elem)) {
                switch(elem.ltype) {
                case ELEMENT_C_FUNC:
                elem.u.cfunc();
                break;
                case EXECUTABLE_ARRAY:
                    eval_exec_array(elem.u.byte_codes);
                break;
                default: {
                    struct Token out_val;
                    if(dict_get(token.u.name, &out_val)) {
                        stack_push(&out_val);
                    } else {
                        stack_push(&token);
                    }
                }
                break;
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
}
static void test_eval_exec_cmp_ops_(char* input, struct Token* expect) {
    cl_getc_set_src(input);
    eval();
    assert_token(stack_pop(), expect);
    assert(stack_pop() == 0);
}

static void test_eval_exec_control_ops() {
    {
    char *input = "{14 23 add} exec";
    struct Token expect = {NUMBER, {.number = 37}};
    cl_getc_set_src(input);
    eval();
    struct Token* actual = stack_pop();
    assert_token(actual, &expect);
    }
    {
    char *inputs[] = {
        "1 {14 23 add} if",
        "0 {14 23 add} if"
    };
    struct Token expect = {NUMBER, {.number = 37}};
    for(int i = 0; i < 2; i++) {
        cl_getc_set_src(inputs[i]);
        eval();
        struct Token* actual = stack_pop();
        if(i == 0) {
            assert_token(actual, &expect);
        } else if(i == 1) {
            assert(actual == 0);
        }
    }
    }
    {
    char *inputs[] = {
        "1 {23 14 add} {23 14 sub} ifelse",
        "0 {23 14 add} {23 14 sub} ifelse"
    };
    struct Token expects[] = {
        {NUMBER, {.number = 37}},
        {NUMBER, {.number = 9}},
    };
    for(int i = 0; i < 2; i++) {
        cl_getc_set_src(inputs[i]);
        eval();
        struct Token* actual = stack_pop();
        assert_token(actual, &expects[i]);
        assert(stack_pop() == 0);
    }
    }
    {
    char *input = "4 {1 2} repeat";
    struct Token expects[] = {
        {NUMBER, {.number = 2}},
        {NUMBER, {.number = 1}},
        {NUMBER, {.number = 2}},
        {NUMBER, {.number = 1}},
        {NUMBER, {.number = 2}},
        {NUMBER, {.number = 1}},
        {NUMBER, {.number = 2}},
        {NUMBER, {.number = 1}},
    };
    cl_getc_set_src(input);
    eval();
    int n = sizeof(expects) / sizeof(struct Token);
    for(int i = 0; i < n; i++) {
        struct Token* actual = stack_pop();
        assert_token(actual, &expects[i]);
    }
    assert(stack_pop() == 0);
    }
    {
    char *input = "/factorial {\n"
                  "dup\n"
                  "%スタックをいつも「途中経過、j」でwhileが評価されるとし、\n"
                  "%jを途中経過に掛けてjを1減らす\n"
                  "{dup 1 gt}\n"
                  "{\n"
                  "1 sub\n"
                  "exch\n"
                  "1 index\n"
                  "mul\n"
                  "exch\n"
                  "} while\n"
                  "pop\n"
                  "} def\n"
                  "3 factorial"
    ;
    struct Token expects[] = {
        {NUMBER, {.number = 6}},
    };
    cl_getc_set_src(input);
    eval();
    int n = sizeof(expects) / sizeof(struct Token);
    for(int i = 0; i < n; i++) {
        struct Token* actual = stack_pop();
        assert_token(actual, &expects[i]);
    }
    assert(stack_pop() == 0);
    }
}
static void test_eval_exec_stack_ops() {
    {
    char *input = "1 2 pop";
    struct Token expect = {NUMBER, {.number= 1}};
    cl_getc_set_src(input);
    eval();
    assert_token(stack_pop(), &expect);
    assert(stack_pop() == 0);
    }
    {
    char *input = "1 2 exch";
    struct Token expects[] = {
        {NUMBER, {.number = 1}},
        {NUMBER, {.number = 2}}
    };
    cl_getc_set_src(input);
    eval();
    int n = sizeof(expects) / sizeof(struct Token);
    for(int i = 0; i < n; i++) {
        struct Token* actual = stack_pop();
        assert_token(actual, &expects[i]);
    }
    assert(stack_pop() == 0);
    }
    {
    char *input = "1 2 dup";
    struct Token expects[] = {
        {NUMBER, {.number = 2}},
        {NUMBER, {.number = 2}},
        {NUMBER, {.number = 1}}
    };
    cl_getc_set_src(input);
    eval();
    int n = sizeof(expects) / sizeof(struct Token);
    for(int i = 0; i < n; i++) {
        struct Token* actual = stack_pop();
        assert_token(actual, &expects[i]);
    }
    assert(stack_pop() == 0);
    }
    {
    char *input = "10 20 30 40 50 2 index";
    struct Token expects[] = {
        {NUMBER, {.number = 30}},
        {NUMBER, {.number = 50}},
        {NUMBER, {.number = 40}},
        {NUMBER, {.number = 30}},
        {NUMBER, {.number = 20}},
        {NUMBER, {.number = 10}},
    };
    cl_getc_set_src(input);
    eval();
    int n = sizeof(expects) / sizeof(struct Token);
    for(int i = 0; i < n; i++) {
        struct Token* actual = stack_pop();
        assert_token(actual, &expects[i]);
    }
    assert(stack_pop() == 0);
    }
    {
    char *input = "1 2 3 4 5 6 7 4 3 roll";
    struct Token expects[] = {
        {NUMBER, {.number = 4}},
        {NUMBER, {.number = 7}},
        {NUMBER, {.number = 6}},
        {NUMBER, {.number = 5}},
        {NUMBER, {.number = 3}},
        {NUMBER, {.number = 2}},
        {NUMBER, {.number = 1}},
    };
    cl_getc_set_src(input);
    eval();
    int n = sizeof(expects) / sizeof(struct Token);
    for(int i = 0; i < n; i++) {
        struct Token* actual = stack_pop();
        assert_token(actual, &expects[i]);
    }
    assert(stack_pop() == 0);
    }
}

static void test_eval_exec_cmp_ops() {
    char *inputs[] = {
        "1 1 eq",
        "1 2 eq",
        "1 2 neq",
        "1 1 neq",
        "2 1 gt",
        "2 2 gt",
        "2 3 gt",
        "1 2 lt",
        "2 2 lt",
        "3 2 lt",
        "2 1 ge",
        "2 2 ge",
        "2 3 ge",
        "1 2 le",
        "2 2 le",
        "3 2 le",
    };
    struct Token expects[] = {
        {NUMBER, {.number= 1}},
        {NUMBER, {.number = 0}},
        {NUMBER, {.number= 1}},
        {NUMBER, {.number = 0}},
        {NUMBER, {.number= 1}},
        {NUMBER, {.number = 0}},
        {NUMBER, {.number= 0}},
        {NUMBER, {.number= 1}},
        {NUMBER, {.number = 0}},
        {NUMBER, {.number= 0}},
        {NUMBER, {.number= 1}},
        {NUMBER, {.number = 1}},
        {NUMBER, {.number= 0}},
        {NUMBER, {.number= 1}},
        {NUMBER, {.number = 1}},
        {NUMBER, {.number= 0}},
    };
    int n = sizeof(inputs)/ sizeof(char*);
    for(int i = 0; i < n; i++) {
        test_eval_exec_cmp_ops_(inputs[i], &expects[i]);
    }
}

static void test_eval_exec_array_exect_array_nest2() {
    char *input = "/ZZ {6} def\n"
                  "/YY {4 ZZ 5} def\n"
                  "/XX {1 2 YY 3} def\n"
                  "XX"
    ;
    struct Token expects[] = {
        {NUMBER, {.number = 3}},
        {NUMBER, {.number = 5}},
        {NUMBER, {.number = 6}},
        {NUMBER, {.number = 4}},
        {NUMBER, {.number = 2}},
        {NUMBER, {.number = 1}}
    };
    cl_getc_set_src(input);
    eval();

    int n = sizeof(expects)/ sizeof(struct Token);
    for(int i = 0; i < n; i++) {
        assert_token(stack_pop(), &expects[i]);
    }
    assert(stack_pop() == 0);
}

static void test_eval_exec_array_exect_array() {
    char *input = "/abc {23 44 add} def\n"
                  "abc"
    ;
    struct Token expect = {NUMBER, {.number = 67}};
    cl_getc_set_src(input);
    eval();

    struct Token* token = stack_pop();
    assert_token(token, &expect);
    assert(stack_pop() == 0);
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
        test_eval_exec_array_nest,
        test_eval_exec_array_exect_array,
        test_eval_exec_array_exect_array_nest2,
        test_eval_exec_cmp_ops,
        test_eval_exec_stack_ops,
        test_eval_exec_control_ops,
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

int main(int argc, char* argv[]) {
    if(argc == 1) {
        unit_tests();
    } else {
        FILE * fp;
        if((fp = fopen(argv[1], "r")) == NULL) {
            return 1;
        }
        cl_getc_set_fp(fp);
        dict_reset();
        stack_reset();
        register_primitives();
        eval();
        fclose(fp);
        stack_print_all();
    }

    return 0;
}
