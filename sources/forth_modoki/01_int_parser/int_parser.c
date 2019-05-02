#include <stdio.h>
#include <assert.h>

static const char* const input = "123 456  1203";

int parse_int(const char** p);

int main() {
    int answer1 = 0;
    int answer2 = 0;
    int answer3 = 0;

    const char* p = input;
    answer1 = parse_int(&p);
    p++;
    answer2 = parse_int(&p);
    p++;
    parse_int(&p);
    p++;
    answer3 = parse_int(&p);
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