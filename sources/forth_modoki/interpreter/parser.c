#include "clesson.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define NAME_SIZE 256
static int isalpha(int ch) {
    return ('a' <= ch && ch <= 'z') || ('A' <= ch && ch <= 'Z');
}
static int isdigit(int ch) {
    return '0' <= ch && ch <= '9';
}
#define skip_white_space(c) while(c == ' ' || c == '\n') c = cl_getc()
static int parse_one_name(int c, struct Token *out_token) {
    char* buf = malloc(sizeof(char) * NAME_SIZE);
    char* p = buf;
    do {
        *p++ = c;
        c =  cl_getc();
    } while(isalpha(c));
    *p = '\0';
    out_token->u.name = buf;
    return c;
}

int parse_one(int prev_ch, struct Token *out_token) {
    int c = prev_ch;
    if(c == EOF)
        c = cl_getc();
    if(c == ' ' || c == '\n') {
        skip_white_space(c);
        out_token->ltype = SPACE;
        out_token->u.onechar = ' ';
        return c;
    }
    if(c == '%') {
        c = cl_getc();
        while(c != '\n' && c != EOF) c = cl_getc();
        out_token->ltype = SPACE;
        out_token->u.onechar = ' ';
        return c;
    }
    if(isdigit(c)) {
        int v = 0;
        do {
            v = v * 10 + c - '0';
            c = cl_getc();
        }while(isdigit(c));
        out_token->ltype = NUMBER;
        out_token->u.number = v;
        return c;
    }
    if(c == '{') {
        out_token->ltype = OPEN_CURLY;
        out_token->u.onechar = c;
        return cl_getc();
    }
    if(c == '}') {
        out_token->ltype = CLOSE_CURLY;
        out_token->u.onechar = c;
        return cl_getc();
    }
    if(c == '/') {
        c = parse_one_name(cl_getc(), out_token);
        out_token->ltype = LITERAL_NAME;
        return c;
    }
    if(isalpha(c)) {
        c = parse_one_name(c, out_token);
        out_token->ltype = EXECUTABLE_NAME;
        return c;
    }
    if(c == EOF) {
        out_token->ltype = END_OF_FILE;
        return c;
    }
    out_token->ltype = UNKNOWN;
    return EOF;
}


void parser_print_all() {
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
                    printf("num: %d\n", token.u.number);
                    break;
                case SPACE:
                    printf("space!\n");
                    break;
                case OPEN_CURLY:
                    printf("Open curly brace '%c'\n", token.u.onechar);
                    break;
                case CLOSE_CURLY:
                    printf("Close curly brace '%c'\n", token.u.onechar);
                    break;
                case EXECUTABLE_NAME:
                    printf("EXECUTABLE_NAME: %s\n", token.u.name);
                    break;
                case LITERAL_NAME:
                    printf("LITERAL_NAME: %s\n", token.u.name);
                    break;

                default:
                    printf("Unknown type %d\n", token.ltype);
                    break;
            }
        }
    }while(ch != EOF);
}

static void test_parse_one_brace() {
    char *input = "{}";
    struct Token token = {UNKNOWN, {0}};
    int ch;
    cl_getc_set_src(input);
    ch = parse_one(EOF, &token);
    assert(ch == '}');
    assert(token.ltype == OPEN_CURLY);
    assert(token.u.onechar == '{');
    ch = parse_one(ch, &token);
    assert(ch == EOF);
    assert(token.ltype == CLOSE_CURLY);
    assert(token.u.onechar == '}');
}

static void test_parse_one_space() {
    char *input = "123   456";
    struct Token token = {UNKNOWN, {0}};
    int ch;

    cl_getc_set_src(input);

    ch = parse_one(EOF, &token);
    assert(ch == ' ');
    assert(token.ltype == NUMBER);
    assert(token.u.number == 123);

    ch = parse_one(ch, &token);
    assert(ch == '4');
    assert(token.ltype == SPACE);
    assert(token.u.onechar == ' ');

    ch = parse_one(ch, &token);
    assert(ch == EOF);
    assert(token.ltype == NUMBER);
    assert(token.u.number == 456);    
}

static void test_parse_one_literal_name() {
    char *input = "/add";
    char *expect_name = "add";
    int expect_type = LITERAL_NAME;

    struct Token token = {UNKNOWN, {0}};
    int ch;

    cl_getc_set_src(input);

    ch = parse_one(EOF, &token);

    assert(ch == EOF);
    assert(token.ltype == expect_type);
    assert(strcmp(token.u.name, expect_name) == 0);
}

static void test_parse_one_executeble_name() {
    char *input = "add";
    char *expect_name = "add";
    int expect_type = EXECUTABLE_NAME;

    struct Token token = {UNKNOWN, {0}};
    int ch;

    cl_getc_set_src(input);

    ch = parse_one(EOF, &token);

    assert(ch == EOF);
    assert(token.ltype == expect_type);
    assert(strcmp(token.u.name, expect_name) == 0);
}

static void test_parse_one_number() {
    char *input = "123";
    int expect = 123;

    struct Token token = {UNKNOWN, {0}};
    int ch;

    cl_getc_set_src(input);

    ch = parse_one(EOF, &token);

    assert(ch == EOF);
    assert(token.ltype == NUMBER);
    assert(expect == token.u.number);
}

static void test_parse_one_empty_should_return_END_OF_FILE() {
    char *input = "";
    int expect = END_OF_FILE;

    struct Token token = {UNKNOWN, {0}};
    int ch;

    cl_getc_set_src(input);
    ch = parse_one(EOF, &token);

    assert(ch == EOF);
    assert(token.ltype == expect);
}


static void unit_tests() {
    test_parse_one_empty_should_return_END_OF_FILE();
    test_parse_one_number();
    test_parse_one_executeble_name();
    test_parse_one_literal_name();
    test_parse_one_brace();
    test_parse_one_space();
}
#if 0
int main() {
    unit_tests();

    cl_getc_set_src("123 45 add /some { 2 3 add } def");
    parser_print_all();
    return 0;
}
#endif