#include "clesson.h"
#include <assert.h>

/*
cc cl_getc.c int_parser_getc.c
*/
enum {
    NUMBER,
    SPACE
};
#define skip_white_space(c) while(c == ' ') c = cl_getc()
int parse_one(int c, int* out_val, int* out_type);

void  test_parse_one_123() {
    int out_val;
    int out_type;
    int c = parse_one(0, &out_val, &out_type);
    assert(out_type == NUMBER);
    assert(out_val = 123);
    assert(c == ' ');
}

void test_parse_one_123_456() {
    int out_val;
    int out_type;
    cl_getc_set_src("123 456");
    int c = parse_one(0, &out_val, &out_type);
    assert(out_type == NUMBER);
    assert(out_val = 123);
    assert(c == ' ');
    c = parse_one(c, &out_val, &out_type);
    assert(out_type == SPACE);
    assert(out_val = ' ');
    assert(c == '4');
    c = parse_one(c, &out_val, &out_type);
    assert(out_type == NUMBER);
    assert(out_val = 456);
    assert(c == EOF);
}

int main() {
    int answer1 = 0;
    int answer2 = 0;
    test_parse_one_123();
    test_parse_one_123_456();
    cl_getc_set_src("123 456");
    int* answers[] = {&answer1, &answer2};
    int i = 0;
    const int n = sizeof(answers)/ sizeof(int*);
    int out_val;
    int out_type;
    int c = 0;
    while(i < n && c != EOF) {
        c = parse_one(c, &out_val, &out_type);
        if(c == 0) return 1;
        switch(out_type) {
        case SPACE:
            break;
        case NUMBER:
            *answers[i++] = out_val;
            break;
        default:
            return 1;
        }
    }

    // verity result.
    assert(answer1 == 123);
    assert(answer2 == 456);

    return 0;


}
int parse_one(int c, int* out_val, int* out_type) {
  if(c == 0)
    c = cl_getc();
  if('0' <= c && c <= '9') {
    int v = 0;
    do {
        v = v * 10 + c - '0';
        c = cl_getc();
    }while('0' <= c && c <= '9');
    *out_type = NUMBER;
    *out_val = v;
    return c;
  }
  if(c == ' ') {
        skip_white_space(c);
        *out_type = SPACE;
        *out_val = ' ';
        return c;
  }
  return 0;
}