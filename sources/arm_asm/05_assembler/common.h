#include <stdio.h>
int cl_getline(char **out_buf);
void cl_getline_set_str(char* str);
void cl_getline_set_fp(FILE* input_fp);
struct substring;
struct Emitter;
struct AsmNode;
int asm_one(char* str, struct AsmNode* node, int addr);
int parse_one(char* str, struct substring* out_subs);
int parse_register(char* str, int* out_register);
int skip_comma(char* str);
void emit_word(struct Emitter* emitter, int oneword);
void assert_strn(const char*  actual, const char* expect, int n);
#define PARSE_FAIL (-100)
#define PARSE_LABEL (-101)
#define MAX_WORDS 100000

struct Node;
int to_mnemonic_symbol(char* str, int len);

void dict_put(const int key, int value);
int dict_get(const int key, int* out_value);
void reset_dict();

void symbol_add(int pos, int label_id);
void resolve_symbols();