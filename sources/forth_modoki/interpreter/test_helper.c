#include "clesson.h"
#include <string.h>
#include <assert.h>
static void setup() {
    dict_reset();
    stack_reset();
    register_primitives();
}
void do_test(void (*test)()) {
    setup();
    test();
}
int streq(const char *s1, const char *s2) {
    return strcmp(s1, s2) == 0;
}
void assert_token(struct Token* actual, struct Token* expect) {
    assert(actual->ltype == expect->ltype);
    switch(actual->ltype) {
        case NUMBER:
            assert(actual->u.number == expect->u.number);
            break;
        case EXECUTABLE_NAME:
        case LITERAL_NAME:
            assert(strcmp(actual->u.name, expect->u.name) == 0);
            break;
        case OPEN_CURLY:
        case CLOSE_CURLY:
        case END_OF_FILE:
            assert(actual->u.onechar == expect->u.onechar);
            break;
        default:
            break;
    }
}