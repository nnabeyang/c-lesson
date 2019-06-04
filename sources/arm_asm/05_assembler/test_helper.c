#include "common.h"
#include <string.h>
#include <assert.h>

void assert_strn(const char*  actual, const char* expect, int n) {
    assert(strncmp(actual, expect, n) == 0);
}