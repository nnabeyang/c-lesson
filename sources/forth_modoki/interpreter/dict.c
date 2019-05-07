#include "clesson.h"
#include <string.h>
#include <assert.h>
#include <stdlib.h>

static int dict_pos = 0;
struct KeyValue {
    const char* key;
    struct Token value;
};
#define DICT_SIZE  1024
static struct KeyValue dict_array[DICT_SIZE];
int streq(const char *s1, const char *s2) {
    return strcmp(s1, s2) == 0;
}

static void assert_token(struct Token* actual, struct Token* expect) {
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
void dict_put(const char* key, struct Token* token) {
    for(int i = 0; i < dict_pos; i++) {
        struct KeyValue* key_value = &dict_array[i];
        if(streq(key, key_value->key)) {
            key_value->value = *token;
            return;
        }     
    }
    struct KeyValue* key_value = &dict_array[dict_pos++];
    char* new_key = malloc(sizeof(char) * strlen(key));
    strcpy(new_key, key);
    key_value->key = new_key;
    key_value->value = *token;
}

int dict_get(const char* key, struct Token* out_token) {
    for(int i = 0; i < dict_pos; i++) {
        struct KeyValue key_value = dict_array[i];
        if(streq(key, key_value.key)) {
            *out_token = key_value.value;
            return 1;
        }
    }
    return 0;
}

void key_value_str(char* buf, struct KeyValue* key_value) {
    switch(key_value->value.ltype) {
        case EXECUTABLE_NAME:
            sprintf(buf, "'%s': '%s'", key_value->key, key_value->value.u.name);
            break;
        case LITERAL_NAME:
            sprintf(buf, "'%s': '/%s'", key_value->key, key_value->value.u.name);
            break;
        default:
            break;
    }
}

void dict_print_all() {
    puts("{");
    for(int i = 0; i < dict_pos; i++) {
        if(i > 0) puts(",");
        struct KeyValue key_value = dict_array[i];
        char out_buf[80];
        key_value_str(out_buf, &key_value);
        printf("%s", out_buf);
    }
    puts("\n}");
}

static void test_dict_put_get_two() {
    struct Token inputs[] = {
     {LITERAL_NAME, {.name = "def"}},
     {LITERAL_NAME, {.name = "abc"}}
     };

    dict_put(inputs[0].u.name, &inputs[0]);
    dict_put(inputs[1].u.name, &inputs[1]);
    struct Token actual;
    assert(!dict_get("no_such_key", &actual));
    assert(dict_get("abc", &actual));
    assert_token(&inputs[1], &actual);

    assert(dict_get("def", &actual));
    assert_token(&inputs[0], &actual);
}

static void test_dict_put_get_one() {
    struct Token input = {LITERAL_NAME, {.name = "def"}};
    dict_put(input.u.name, &input);
    struct Token actual;
    assert(!dict_get("no_such_key", &actual));
    assert(dict_get("def", &actual));
    assert_token(&input, &actual);
}

static void test_dict_get_empty_case() {
    struct Token actual;
    assert(!dict_get("key", &actual));
}

static void dict_unit_tests() {
    test_dict_get_empty_case();
    dict_pos = 0;
    test_dict_put_get_one();
    dict_pos = 0;
    test_dict_put_get_two();
}

int main() {
    dict_unit_tests();
    dict_pos = 0;
    struct Token inputs[] = {
     {EXECUTABLE_NAME, {.name = "def"}},
     {LITERAL_NAME, {.name = "abc"}}
     };

    dict_put(inputs[0].u.name, &inputs[0]);
    dict_put(inputs[1].u.name, &inputs[1]);
    dict_print_all();
    return 0;
}