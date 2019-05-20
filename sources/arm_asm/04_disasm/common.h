#include <stdarg.h>
#include <stdio.h>

void cl_clear_output();
char *cl_get_result(int num);
void cl_enable_buffer_mode();
void cl_disable_buffer_mode();
void cl_printf(char *fmt, ...);

int print_asm(int word);