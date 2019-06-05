#include <string.h>
#include "common.h"
#define BUF_SIZE 1024
static char buf[BUF_SIZE];
static const char* input;
static int pos = 0;
static FILE* fp = NULL;
int cl_getline(char **out_buf) {
    if(fp != NULL) {
        size_t n;
        return getline(out_buf, &n, fp);
    }
    char *p = strrchr(&input[pos], '\n');
    int len;
    if(p != NULL) {
        len = p - &input[pos] + 1; 
        strncpy(buf, &input[pos], len);
    } else {
        strcpy(buf, &input[pos]);
    }
    *out_buf = buf;
    pos += len;
    return len;
}

void cl_getline_set_str(char* str) {
    input = str;
    pos = 0;
}
void cl_getline_set_fp(FILE* input_fp) {
    fp = input_fp;
    pos = 0;
}