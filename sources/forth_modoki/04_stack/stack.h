#define STACK_SIZE 1024
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
struct Stack {
    int n;
    struct Token* tokens[1];    
};

void stack_print_all(struct Stack* stack);
void stack_push(struct Stack* stack, struct Token* token);
struct Token* stack_pop(struct Stack* stack);