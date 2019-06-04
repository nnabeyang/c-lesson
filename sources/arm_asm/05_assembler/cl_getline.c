#include <string.h>
#include "common.h"
#include <stdio.h>
#define BUF_SIZE 1024
static char buf[BUF_SIZE];
static const char* input;
static int pos = 0;
int cl_getline(char **out_buf) {
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