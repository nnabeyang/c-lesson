#include "clesson.h"
#include <string.h>
#include <assert.h>
#include <stdlib.h>
struct KeyNode {
    char* key;
    struct KeyNode* next;
};

#define DICT_SIZE  16
static struct KeyValue* dict_array[DICT_SIZE];
static struct KeyNode keys = {NULL, NULL};
static struct KeyNode* tail = &keys;

int hash(const char* str) {
    unsigned int val = 0;
    while(*str) {
        val += *str++;
    }
    return (int)(val % DICT_SIZE);
}

void update_or_insert_list(struct KeyValue* head, const char* key, struct Token* token) {
    struct KeyValue* p = head;
    struct KeyValue* lastNode;
    do {
        if(streq(p->key, key)) {
            p->value = *token;
            return;
        }
        lastNode = p;
        p = p->next;
    }while(p != NULL);

    struct KeyValue* val = malloc(sizeof(struct KeyValue));
    val->next = NULL;
    val->key = malloc(sizeof(char) * strlen(key));
    strcpy(val->key, key);
    val->value = *token;
    lastNode->next = val;

    struct KeyNode* node = malloc(sizeof(struct KeyNode));
    node->key = val->key;
    node->next = NULL;
    tail->next = node;
    tail = node;
}

void dict_put(const char* key, struct Token* token) {
    int idx = hash(key);
    struct KeyValue* head = dict_array[idx];
    if(head == NULL) {
        head = malloc(sizeof(struct KeyValue));
        head->next = NULL;
        head->key = malloc(sizeof(char) * strlen(key));
        strcpy(head->key, key);
        head->value = *token;

        dict_array[idx] = head;
        assert(tail->next == NULL);
        struct KeyNode* node = malloc(sizeof(struct KeyNode));
        node->key = head->key;
        node->next = NULL;
        tail->next = node;
        tail = node;
        return;
    }
    update_or_insert_list(head, key, token);
}

int dict_get(const char* key, struct Token* out_token) {
    int idx = hash(key);
    struct KeyValue* head = dict_array[idx];
    while(head != NULL) {
        if(streq(head->key, key)) {
            *out_token = head->value;
            return 1;
        }
        head = head->next;
    }
    return 0;
}

void dict_reset() {
    struct KeyNode* p = keys.next;
    while(p != NULL) {
        struct KeyNode* next = p->next;
        int idx = hash(p->key);
        dict_array[idx] = NULL;
        p->next = NULL;
        p = next;
    }
    keys.next = NULL;
    tail = &keys;
}

void key_value_str(char* buf, char* key, struct Token* token) {
    switch(token->ltype) {
        case NUMBER:
              sprintf(buf, "'%s': %d", key, token->u.number);
              break;
        case EXECUTABLE_NAME:
            sprintf(buf, "'%s': '%s'", key, token->u.name);
            break;
        case LITERAL_NAME:
            sprintf(buf, "'%s': '/%s'", key, token->u.name);
            break;
        default:
            break;
    }
}

void dict_print_all() {
    puts("{");
    struct KeyNode* p = keys.next;
    int i = 0;
    while(p != NULL) {
        if(i > 0) puts(",");
        struct Token out_value;
        dict_get(p->key, &out_value);
        char out_buf[80];
        key_value_str(out_buf, p->key, &out_value);
        printf("%s", out_buf);
        p = p->next;
        i++;
    }
    puts("\n}");
}

static void test_same_hash_values() {
    char* hash_keys[] = {
      "adf",
      "aee",
      "acg"
    };
    assert(hash(hash_keys[0]) == hash(hash_keys[1]));
    assert(hash(hash_keys[0]) == hash(hash_keys[2]));
    struct Token tokens[] = {
        {EXECUTABLE_NAME, {.name = "xxxx"}},
        {LITERAL_NAME, {.name = "yyyy"}},
        {NUMBER, {.number = 123}}
    };
    int n = sizeof(hash_keys) / sizeof(char*);
    for(int i = 0; i < n; i++) {
        dict_put(hash_keys[i], &tokens[i]);
    }

    int idx = hash(hash_keys[0]);
    struct KeyValue * p = dict_array[idx];

    assert(streq("adf", p->key));
    assert_token(&tokens[0], &p->value);

    p = p->next;
    assert(streq("aee", p->key));
    assert_token(&tokens[1], &p->value);

    p = p->next;
    assert(streq("acg", p->key));
    assert_token(&tokens[2], &p->value);
    assert(p->next == NULL);

    dict_reset();
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
    dict_reset();
}

static void test_dict_put_get_one() {
    struct Token input = {LITERAL_NAME, {.name = "def"}};
    dict_put(input.u.name, &input);
    struct Token actual;
    assert(!dict_get("no_such_key", &actual));
    assert(dict_get("def", &actual));
    assert_token(&input, &actual);
    dict_reset();
}

static void test_dict_get_empty_case() {
    struct Token actual;
    assert(!dict_get("key", &actual));
    dict_reset();
}

void dict_unit_tests() {
    test_dict_get_empty_case();
    test_dict_put_get_one();
    test_dict_put_get_two();
    test_same_hash_values();
}

#if 0
int main() {
    dict_unit_tests();
    struct Token inputs[] = {
     {EXECUTABLE_NAME, {.name = "def"}},
     {LITERAL_NAME, {.name = "abc"}}
     };

    dict_put("key1", &inputs[0]);
    dict_put("key2", &inputs[1]);
    dict_print_all();
    dict_reset();
    return 0;
}
#endif