#include <stdio.h>

/*
return one character and move cursor.
return EOF if end of file.
*/
int cl_getc();
void cl_getc_set_src(char* str);
void cl_getc_set_fp(FILE* input_fp);

enum LexicalType {
    NUMBER,
    SPACE,
    EXECUTABLE_NAME,
    LITERAL_NAME,
    ELEMENT_C_FUNC,
    EXECUTABLE_ARRAY,
    OPEN_CURLY,
    CLOSE_CURLY,
    END_OF_FILE,
    UNKNOWN
};

struct ElementArray;
struct Token {
    enum LexicalType ltype;
    union {
        int number;
        char onechar;
        char *name;
        void (*cfunc)();
        struct ElementArray* byte_codes;
    } u;
};

struct ElementArray {
    int len;
    struct Token elements[0];
};

int parse_one(int prev_ch, struct Token *out_token);
void parser_print_all();

void stack_print_all();
void stack_push(struct Token* token);
struct Token* stack_pop();
void stack_reset();

struct KeyValue {
    char* key;
    struct Token value;
    struct KeyValue *next;
};

void dict_put(const char* key, struct Token* token);
int dict_get(const char* key, struct Token* out_token);
void dict_print_all();
void dict_reset();
int hash(const char* str);

void register_primitives();
void do_test(void (*test)());
int streq(const char *s1, const char *s2);
void assert_token(struct Token* actual, struct Token* expect);
void dict_unit_tests();
void stack_unit_tests();
