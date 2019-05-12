#include "clesson.h"
#include <string.h>

static const char* input = "123 456";
static int pos = 0;
static FILE* fp = NULL;
static int eof_flag = 0;
int cl_getc() {
    if(fp != NULL) {
        int c = fgetc(fp);
        if(!eof_flag) {
            eof_flag = c == EOF;
        } else {
            return EOF;
        }
        return c;
    }
    if(strlen(input) == pos)
        return EOF;
    return input[pos++];
}

void cl_getc_set_fp(FILE* input_fp) {
    fp = input_fp;
}
void cl_getc_set_src(char* str){
    input = str;
    pos = 0;
}
