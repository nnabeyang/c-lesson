#include "clesson.h"
#include <assert.h>

/*
cc cl_getc.c int_parser_getc.c
*/
enum {
    NUMBER,
    SPACE,
    UNKNOWN
};
#define skip_white_space(c) while(c == ' ') c = cl_getc()
int parse_one(int c, int* out_val, int* out_type);

int main() {
    int answer1 = 0;
    int answer2 = 0;
    int* answers[] = {&answer1, &answer2};
    int i = 0;
    const int n = sizeof(answers)/ sizeof(int*);
    int out_val;
    int out_type;
    int c = cl_getc();
    while(i < n && c != EOF) {
        c = parse_one(c, &out_val, &out_type);
        switch(out_type) {
        case SPACE:
            break;
        case NUMBER:
            *answers[i++] = out_val;
            break;
        case UNKNOWN:
            fprintf(stderr, "' ' or [0-9] is expected, but got '%c'\n", c);
            return 1;
        }
    }

    // verity result.
    assert(answer1 == 123);
    assert(answer2 == 456);

    return 0;


}
int parse_one(int c, int* out_val, int* out_type) {
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
        return c;
  }
  *out_type = UNKNOWN;
  return c;
}