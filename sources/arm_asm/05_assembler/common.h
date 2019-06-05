int cl_getline(char **out_buf);
void cl_getline_set_str(char* str);
struct substring;
struct Emitter;
int asm_one(char* str);
int parse_one(char* str, struct substring* out_subs);
int parse_register(char* str, int* out_register);
int skip_comma(char* str);
void emit_word(struct Emitter* emitter, int oneword);
void assert_strn(const char*  actual, const char* expect, int n);
#define PARSE_FAIL (-100)
#define MAX_WORDS 100000