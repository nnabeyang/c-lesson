#include <assert.h>
#include <string.h>
#include "common.h"
#include <stdlib.h>

enum StrState {
  START,
  STR,
  ESCAPE,
  END
};

int parse_string(char* buf, char** out_str) {
  static char tmpbuf[1024];
  char* p = tmpbuf;
  int i = 0;
  enum StrState state =  START;
  while(state != END) {
    char ch = buf[i++];
    if(ch == '\0') {
      fprintf(stderr, "this is not closed double quote.\n");
      return PARSE_FAIL;
    }
    switch(state) {
      case START:
        assert(ch == '"');
        state = STR;
        continue;
      case STR:
        if(ch == '\\') state = ESCAPE;
        else if(ch == '"') state = END;
        else {
          *p++ = ch;
        }
        continue;
      case ESCAPE:
        if(ch == 'n') *p++ = '\n';
        if(ch == '"') *p++ = '\"';
        if(ch == '\\') *p++ = '\\';
        state = STR;
        continue;
      default:
        fprintf(stderr, "parse failed\n");
        return PARSE_FAIL;      
    }
  }
  *p = '\0';
  size_t size = p - tmpbuf;
  char* res = (char*) malloc(size);
  memcpy(res, tmpbuf, size);
  *out_str = res;
  return size;
}
struct SymbolNode {
  int pos;
  int label_id;
  struct SymbolNode* next;
};
static struct SymbolNode symbolRoot;
static struct SymbolNode* symbols = &symbolRoot;
void reset_unresolve_list() {
  symbols = &symbolRoot;
}
struct substring {
  char* str;
  int len;
};
struct Emitter {
  int* elems;
  int pos;
};
struct Node {
  char* name;
  int value;
  struct Node* left;
  struct Node* right;
};

struct KeyValue {
  int key;
  int value;
  struct KeyValue *next;
};

#define DICT_SIZE  1024
static struct KeyValue dict_array[DICT_SIZE];
static int dict_pos = 0;


void dict_put(const int key, int value) {
  for(int i = 0; i < dict_pos; i++) {
    struct KeyValue* key_value = &dict_array[i];
    if(key == key_value->key) {
      key_value->value = value;
      return;
    }
  }
  struct KeyValue* key_value = &dict_array[dict_pos++];
  key_value->key = key;
  key_value->value = value;
}
int dict_get(const int key, int* out_value) {
  for(int i = 0; i < dict_pos; i++) {
    struct KeyValue key_value = dict_array[i];
    if(key == key_value.key) {
      *out_value = key_value.value;
      return 1;
    }
  }
  return 0;
}
void reset_dict() {
  dict_pos = 0;
}

struct Node mnemonic_root;
struct Node label_root;

int mnemonic_id = 0;
int label_id = 10000;

struct Node* create_node(char* name, int len, int value) {
  struct Node* node = malloc(sizeof(struct Node));
  node->name = malloc(sizeof(char) * len);
  node->left = NULL;
  node->right = NULL;
  strncpy(node->name, name, len);
  node->value = value;
  return node;
}

int to_mnemonic_symbol_of(struct Node* parent, char* str, int len, int* id) {
  if(parent->name == NULL) {
      parent->name = malloc(sizeof(char) * len);
      strncpy(parent->name, str, len);
      parent->value = (*id)++;
      return parent->value;
  }
  int cmp = strncmp(parent->name, str, len);
  if(cmp == 0) return parent->value;
  if(cmp > 0) {
    if(parent->right == NULL) {
      parent->right = create_node(str, len, (*id)++);
      return parent->right->value;
    }
    return to_mnemonic_symbol_of(parent->right, str, len, id);
  }
  if(cmp < 0) {
     if(parent->left == NULL) {
      parent->left = create_node(str, len, (*id)++);
      return parent->left->value;
    }
    return to_mnemonic_symbol_of(parent->left, str, len, id);
  }
  return -1;
}

int to_mnemonic_symbol(char* str, int len) {
  return to_mnemonic_symbol_of(&mnemonic_root, str, len, &mnemonic_id);
}

int to_label_symbol(char* str, int len) {
  return to_mnemonic_symbol_of(&label_root, str, len, &label_id);
}

void reset_symbols() {
  mnemonic_root.name = NULL;
  mnemonic_root.left = NULL;
  mnemonic_root.right = NULL;
  label_root.name = NULL;
  label_root.left = NULL;
  label_root.right = NULL;
  mnemonic_id = 0;
  label_id = 10000;
}

void setup_symbols() {
  to_mnemonic_symbol("b", 1);
  to_mnemonic_symbol("mov", 3);
  to_mnemonic_symbol(".raw", 4);
  to_mnemonic_symbol("ldr", 3);
  to_mnemonic_symbol("str", 3);
}

static int is_space(int ch) {
  return ch == ' ' || ch == '\t'  || ch == '\n';
}
static int is_digit(int ch) {
  return ('0' <= ch && ch <= '9');
}
static int is_hex(int ch) {
  return is_digit(ch) || ('a' <= ch && ch <= 'f') || ('A' <= ch && ch <= 'F');
}
static int is_capital(int ch) {
  return 'A' <= ch && ch <= 'Z';
}
static int is_alnum(int ch) {
  return is_digit(ch) || ('a' <= ch && ch <= 'z') || ('A' <= ch && ch <= 'Z');
}
static int is_register(int ch) {
  return ch == 'r';
}

int parse_one(char* str, struct substring* out_subs) {
  int pos = 0;
  if(str[pos] == '\0') return 0;
  if(str[pos] == ':') {
      out_subs->str = str;
      out_subs->len = 1;
      return 1;
  }
  if(is_space(str[pos])) {
    while(is_space(str[pos++]));
    out_subs->str = str;
    out_subs->len = pos - 1;
    return pos - 1;
  }
  if(str[pos] == '.' || is_alnum(str[pos])) {
    while(str[pos] == '.' || is_alnum(str[pos]) || str[pos] == '_') pos++;
    out_subs->str = str;
    out_subs->len = pos;
    return pos;
  }
  return PARSE_FAIL;
}

int parse_register(char* str, int* out_register) {
  int  pos = 0;
  while(is_space(str[pos])) pos++;
  if(str[pos] == 'r') {
    int v = 0;
    int c = str[++pos];
    do {
      v = v * 10 + c - '0';
      c = str[++pos];
    }while(is_digit(c));
    *out_register = v;
    return pos;
  } else {
    return PARSE_FAIL;
  }
}

int parse_hex(char* str, int* out_value) {
  int  pos = 0;
  while(is_space(str[pos])) pos++;
  int is_negative = 0;
  if(str[pos] == '-') {
    is_negative = 1;
    pos++;
  }
  if(strncmp(&str[pos], "0x", 2) != 0) return PARSE_FAIL;
  pos += 2;
  int v = 0;
  int c = str[pos];
  while(is_hex(c)) {
    if(is_digit(c)) {
      v = v * 16 + c - '0';
    } else if(is_capital(c)) {
      v = v * 16 + c - 'A' + 10;
    } else {
      v = v * 16 + c - 'a' + 10;
    }
    c = str[++pos];
  }
  *out_value = (is_negative)? -v : v;
  return pos;
}

int parse_immediate(char* str, int* out_value) {
  int  pos = 0;
  while(is_space(str[pos])) pos++;
  if(strncmp(&str[pos], "#", 1) != 0) return PARSE_FAIL;
  return parse_hex(&str[++pos], out_value) + 1;
}

int skip_symbol(char* str, int symbol) {
  int pos = 0;
  while(is_space(str[pos])) pos++;
  if(str[pos] == symbol) return pos + 1;
  else return PARSE_FAIL;
}

int asm_str(char* str, int* out_word) {
  int n, rd, rn;
  n = parse_register(str, &rn);
  if(n == PARSE_FAIL) return PARSE_FAIL;
  str += n;
  n = skip_symbol(str, ',');
  if(n == PARSE_FAIL) return PARSE_FAIL;
  str += n;
  n = skip_symbol(str, '[');
  if(n == PARSE_FAIL) return PARSE_FAIL;
  str += n;
  n = parse_register(str, &rd);
  if(n == PARSE_FAIL) return PARSE_FAIL;
  str += n;
  n = skip_symbol(str, ']');
  if(n == PARSE_FAIL) return PARSE_FAIL;

  *out_word = 0xE5800000  + (rd << 16) + (rn << 12);
  return 1;
}

int asm_ldr(char* str, int* out_word) {
  int n, rd, rn;
  int offset = 0;
  n = parse_register(str, &rn);
  if(n == PARSE_FAIL) return PARSE_FAIL;
  str += n;
  n = skip_symbol(str, ',');
  if(n == PARSE_FAIL) return PARSE_FAIL;
  str += n;
  n = skip_symbol(str, '[');
  if(n == PARSE_FAIL) return PARSE_FAIL;
  str += n;
  n = parse_register(str, &rd);
  if(n == PARSE_FAIL) return PARSE_FAIL;
  str += n;
  n = skip_symbol(str, ']');
  if(n == PARSE_FAIL) {
  n = skip_symbol(str, ',');
  if(n == PARSE_FAIL) return PARSE_FAIL;
    str += n;
    while(is_space(str[0])) str++;
    n = parse_immediate(str, &offset);
    if(n == PARSE_FAIL) return PARSE_FAIL;
    str += n;
    n = skip_symbol(str, ']');
    if(n == PARSE_FAIL) return PARSE_FAIL;
  }
  *out_word = 0xE5100000 + ((offset >= 0) << 23) + (rd << 16) + (rn << 12) + abs(offset);
  return 1;
}

static void str_to_word(char* str, int* out_word) {
  int v = 0;
  for(int i = 0; i < 4; i++) {
    int ch = str[i];
    if(ch == '\0') break;    
    v += ch << (i * 8);
  }
  *out_word = v;
}
int asm_raw(char* str, int* out_word) {
  int n, v;
  int  pos = 0;
  while(is_space(str[pos])) pos++;
  str += pos;
  if(str[0] == '"') {
    char* out_str;
    n = parse_string(str, &out_str);
    str_to_word(out_str, &v);
  } else {
    n = parse_hex(str, &v);
  }
  if(n == PARSE_FAIL) return PARSE_FAIL;
  *out_word = v;
  return 1;
}

int asm_mov(char* str, int* out_word) {
  int n, r1, rm;
  n = parse_register(str, &r1);
  if(n == PARSE_FAIL) return PARSE_FAIL;
  str += n;
  n = skip_symbol(str, ',');
  if(n == PARSE_FAIL) return PARSE_FAIL;
  str += n;
  while(is_space(str[0])) str++;
  int is_immediate = !is_register(str[0]);
  if(!is_immediate) {
    n = parse_register(str, &rm);
  } else {
    n = parse_immediate(str, &rm);
  }
  if(n == PARSE_FAIL) return PARSE_FAIL;
  *out_word = 0xE1A00000 + (r1 << 12) + rm + (is_immediate << 25);
  return 1;
}

int asm_b(char* str, int* out_word, int addr) {
  int n;
  struct substring out_subs = {0};
  int pos = 0;
  while(is_space(str[pos])) pos++;
  str += pos;
  n = parse_one(str, &out_subs);
  if(n == PARSE_FAIL) return n;
  int label_id = to_label_symbol(out_subs.str, out_subs.len);
  symbol_add(addr, label_id);
  *out_word = 0xEA000000;
  return 1;
}

int asm_one(char* str, int* out_word, int addr) {
  int n;
  struct substring out_subs = {0};
  int r1, rm;
  n = parse_one(str, &out_subs);
  if(n == PARSE_FAIL) return PARSE_FAIL;
  str += n;
  if(str[0] == ':') {
    int id = to_label_symbol(out_subs.str, out_subs.len);
    dict_put(id, addr);
    return PARSE_LABEL;
  }
  int symbol = to_mnemonic_symbol(out_subs.str, out_subs.len);
  switch(symbol) {
  case 0:
    return asm_b(str, out_word, addr);
  case 1:
    return asm_mov(str, out_word);
  case 2:
    return asm_raw(str, out_word);
  case 3:
    return asm_ldr(str, out_word);
  case 4:
    return asm_str(str, out_word);
  default:
    return PARSE_FAIL;
  }
}

static int array[MAX_WORDS];

void emit_word(struct Emitter* emitter, int oneword) {
  array[emitter->pos++] = oneword;
}
void print_words(struct Emitter* emitter) {
  for(int i = 0; i < emitter->pos; i++) {
    printf("0x%X\n", array[i]);
  }
}
void save_words(struct Emitter* emitter) {
  FILE* fp;
  if ((fp = fopen("./a.bin", "wb+")) == NULL) {
        return ;
  }
  for(int i = 0; i < emitter->pos; i++) {
    fwrite(&array[i], sizeof(array[i]), 1, fp);
  }
  fclose(fp);
}

void symbol_add(int pos, int label_id) {
  struct SymbolNode* v = malloc(sizeof(struct SymbolNode));
  v->pos = pos;
  v->label_id = label_id;
  v->next = NULL;
  symbols->next = v;
  symbols = v;
}

void resolve_symbols() {
  struct SymbolNode* p = &symbolRoot;
  while(p->next != NULL) {
    p = p->next;
    int addr;
    if(dict_get(p->label_id, &addr) == 1) {
      int r_addr = addr - p->pos - 1;
      int word = array[p->pos];
      array[p->pos] = word | (0XFFFFFF + r_addr);
    }
  }
}

static void test_parse_string() {
  char* out_buf;
  assert(parse_string("\"hello, world\"", &out_buf) == 12);
  assert(strcmp(out_buf, "hello, world") == 0);
  
  assert(parse_string("\"End with back slash. \\\\\"", &out_buf) == 22);
  assert(strcmp(out_buf, "End with back slash. \\") == 0); 

  assert(parse_string("\"Back slash and double quote \\\\\\\"", &out_buf) == PARSE_FAIL);

  assert(parse_string("\"This is \\\" double qouote!\"", &out_buf) == 24);
  assert(strcmp(out_buf, "This is \" double qouote!") == 0);  
}

static void test_b() {
  int out_word;
  reset_symbols();
  reset_dict();
  setup_symbols();
  int addr = 0;
  struct Emitter emitter = {0};
  emitter.elems = array;
  assert(asm_one("loop:\n", &out_word, 0) == PARSE_LABEL);
  assert(asm_one("mov r1, r2\n", &out_word, 0) == 1);
  emit_word(&emitter, out_word);
  assert(asm_one("b loop\n", &out_word, 1) == 1);
  emit_word(&emitter, out_word);
  resolve_symbols();
  assert(array[1] == 0XEAFFFFFD);
}

static void test_setup_symbols() {
  reset_symbols();
  setup_symbols();
  int id = to_mnemonic_symbol("b loop", 1);
  assert(id == 0);
}
static void test_dict() {
  reset_dict();
  int expect1 = 0x12345678;
  int expect2 = 0x2468abcd;
  dict_put(1, expect1);
  dict_put(2, expect2);
  int out_val;
  assert(dict_get(3, &out_val) == 0);
  assert(dict_get(2, &out_val) == 1);
  assert(out_val == expect2);
  assert(dict_get(1, &out_val) == 1);
  assert(out_val == expect1);
  reset_dict();
}
static void test_label() {
  int out_word;
  reset_symbols();
  reset_dict();
  int addr = 0;
  assert(asm_one("loop:\n", &out_word, ++addr) == PARSE_LABEL);
  assert(asm_one("sum:\n", &out_word, ++addr) == PARSE_LABEL);
  assert(asm_one("mov:\n", &out_word, ++addr) == PARSE_LABEL);
  assert(to_label_symbol("loop", 4) == 10000);
  int out;
  assert(dict_get(10000, &out) == 1);
  assert(out == 1);
  assert(to_label_symbol("sum", 3) == 10001);
  assert(dict_get(10001, &out) == 1);
  assert(out == 2);
  assert(to_label_symbol("mov", 3) == 10002);
  assert(dict_get(10002, &out) == 1);
  assert(out == 3);
  reset_symbols();
}

static void test_to_label_symbol() {
  assert(to_label_symbol("loop", 4) == 10000);
  assert(to_label_symbol("print", 5) == 10001);
  assert(to_label_symbol("end", 3) == 10002);
  assert(to_label_symbol("print", 5) == 10001);
  assert(to_label_symbol("loop", 4) == 10000);
  assert(to_label_symbol("end", 3) == 10002);
  reset_symbols();
}

static void test_to_mnemonic_symbol() {
  assert(to_mnemonic_symbol("mov", 3) == 1);
  assert(to_mnemonic_symbol("str", 3) == 4);
  assert(to_mnemonic_symbol(".raw", 4) == 2);
    assert(to_mnemonic_symbol("ldr", 3) == 3);
  reset_symbols();
}

static void test_str() {
  int out_word;
  assert(asm_one("str r0, [r1]\n", &out_word, 0) != PARSE_FAIL);
  assert(out_word == 0xE5810000);
}
static void test_ldr() {
  int out_word;
  assert(asm_one("ldr r1, [r15, #0x30]\n", &out_word, 0) != PARSE_FAIL);
  assert(out_word == 0xE59F1030);
  assert(asm_one("ldr r1, [r15, #-0x30]\n", &out_word, 0) != PARSE_FAIL);
  assert(out_word == 0xE51F1030);
  assert(asm_one("ldr r1, [r15]\n", &out_word, 0) != PARSE_FAIL);
  assert(out_word == 0xE59F1000);
}

static void test_raw_hex() {
  int out_word;
  assert(asm_one(".raw 0x12345678\n", &out_word, 0) != PARSE_FAIL);
  assert(out_word == 0x12345678);
}

static void test_raw_str() {
  int out_word;
  assert(asm_one(".raw \"test\"\n", &out_word, 0) != PARSE_FAIL);
  assert(out_word == 0x74736574);
}

static void test_str_to_word() {
  int out_word;
  str_to_word("test", &out_word);
  assert(out_word == 0x74736574);
  str_to_word("abc", &out_word);
  assert(out_word == 0x00636261);
}

static void test_parse_immediate() {
  int v;
  parse_immediate(" #0x68", &v);
  assert(v == 0x68);
}

static void test_parse_immediate_negative() {
  int v;
  parse_immediate(" #-0x30", &v);
  assert(v == -0x30);
}

static void test_parse_register() {
  char *buf = "mov r1, r2";
  int r1, r2;
  buf += 4;
  assert(parse_register(buf, &r1) == 2);
  assert(r1 == 1);
  buf += 2;
  assert(skip_symbol(buf, ',') == 1);
  buf += 1;
  assert(parse_register(buf, &r2) == 2);
  assert(r2 == 2);
}
static void test_asm_one() {
  int out_word;
  assert(asm_one("mov r1, r2\n", &out_word, 0) != PARSE_FAIL);
  assert(out_word == 0xE1A01002);
  assert(asm_one("mov r2, #0x68\n", &out_word, 0) != PARSE_FAIL);
  assert(out_word == 0xE3A02068);
}

static void test_parse_one() {
  struct substring out_subs;
  int n;
  n = parse_one("mov  r1, r2", &out_subs);
  assert(n == 3);
  assert_strn(out_subs.str, "mov", 3);
}

static void test_parse_one_label() {
  struct substring out_subs;
  int n;
  n = parse_one("loop: \n", &out_subs);
  assert(n == 4);
  assert_strn(out_subs.str, "loop", 4);
}

static void test_getline() {
    cl_getline_set_str("mov r1, r2\n"
                            "mov r1, #0x68");
    char *buf = (char*)malloc(sizeof(char) * 80);    
    cl_getline(&buf);
    assert(strcmp(buf, "mov r1, r2\n") == 0);
    cl_getline(&buf);
    assert(strcmp(buf, "mov r1, #0x68") == 0);  
}

void unit_tests() {
  test_getline();
  test_parse_one();
  test_parse_immediate();
  test_asm_one();
  test_raw_hex();
  test_ldr();
  test_parse_immediate_negative();
  test_str();
  test_to_mnemonic_symbol();
  test_to_label_symbol();
  test_parse_one_label();
  test_label();
  test_dict();
  test_setup_symbols();
  test_b();
  test_parse_string();
  test_raw_str();
  test_str_to_word();
}

int main(int argc, char* argv[]) {
  setup_symbols();
  if(argc == 1) {
    unit_tests();
  } else {
    FILE* fp;
    if((fp = fopen(argv[1], "r")) == NULL) {
      return 1;
    }
    cl_getline_set_fp(fp);
    char* out_char = malloc(sizeof(char) * 1024);
    struct Emitter emitter = {0};
    emitter.elems = array;
    int addr = 0;
    while(cl_getline(&out_char) != -1) {
      int word;
      int result = asm_one(out_char, &word, addr);
      if(result == PARSE_FAIL) {
        fprintf(stderr, "%s: parse failed\n", out_char);
        return PARSE_FAIL;
      }
      if(result != PARSE_LABEL) {
        emit_word(&emitter, word);
        addr++;
      }
    }
    resolve_symbols();
    save_words(&emitter);
    print_words(&emitter);
  }
  return 0;
}