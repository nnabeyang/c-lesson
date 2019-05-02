#include <stdio.h>
#include <assert.h>

static const char* const input = "123 456  1203";

int parse_int(const char** p);
#define skip_white_space(p) while(*p == ' ') p++

int main() {
    int answer1 = 0;
    int answer2 = 0;
    int answer3 = 0;
    int* answers[] = {&answer1, &answer2, &answer3};
    const char* p = input;
    int n = sizeof(answers) / sizeof(int*);
    int i = 0;
    while(i < n && *p != '\0') {
        skip_white_space(p);
        *answers[i++] = parse_int(&p);
    }
    // verity result.
    assert(answer1 == 123);
    assert(answer2 == 456);
    assert(answer3 == 1203);

    return 1;
}

int parse_int(const char** p) {
    int v = 0;
    const char* c = *p;
    while('0' <= *c && *c <= '9') {
        v = v * 10 + (*c++ - '0');
    }
    *p = c;
    return v;
}