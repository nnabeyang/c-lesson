#include "clesson.h"
#include <assert.h>

/*
cc cl_getc.c int_parser_getc.c
*/
#define skip_white_space(c) while(c == ' ') c = cl_getc()
int parse_int(int c);

int main() {
    int answer1 = 0;
    int answer2 = 0;
    int* answers[] = {&answer1, &answer2};
    int c;
    int i = 0;
    const int n = sizeof(answers)/ sizeof(int*);
    while(i < n && (c = cl_getc()) != EOF) {
        skip_white_space(c);
        *answers[i++] = parse_int(c);
    }

    // verity result.
    assert(answer1 == 123);
    assert(answer2 == 456);

    return 1;


}

int parse_int(int c) {
    int v = 0;
    while('0' <= c && c <= '9') {
        v = v * 10 + c - '0';
        c = cl_getc();
    }
    return v;
}