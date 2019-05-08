#include <stdio.h>

/*
return one character and move cursor.
return EOF if end of file.
*/
int cl_getc();
void cl_getc_set_src(char* str);

enum LexicalType {
    NUMBER,
    SPACE,
    EXECUTABLE_NAME,
    LITERAL_NAME,
    OPEN_CURLY,
    CLOSE_CURLY,
    END_OF_FILE,
    UNKNOWN
};

struct Token {
    enum LexicalType ltype;
    union {
        int number;
        char onechar;
        char *name;
    } u;
};

int parse_one(int prev_ch, struct Token *out_token);
void parser_print_all();

struct Stack {
    int n;
    struct Token tokens[1];
};

void stack_print_all(struct Stack* stack);
void stack_push(struct Stack* stack, struct Token* token);
struct Token* stack_pop(struct Stack* stack);
struct Stack* new_stack();

struct KeyValue {
    char* key;
    struct Token value;
    struct KeyValue *next;
};

void dict_put(const char* key, struct Token* token);
int dict_get(const char* key, struct Token* out_token);
void dict_print_all();
void reset_dict();
int hash(const char* str);

int streq(const char *s1, const char *s2);
void assert_token(struct Token* actual, struct Token* expect);
void dict_unit_tests();
void stack_unit_tests();
