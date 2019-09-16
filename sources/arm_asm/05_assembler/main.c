#include <assert.h>
#include <string.h>
#include "common.h"
#include <stdlib.h>

#define skip_space(str) while(is_space(*str)) str++
enum AsmType {
  WORD,
  RAW
};

struct AsmNode {
  enum AsmType type;
  union {
    int word;
    char* str;
  } u;
};

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
  symbolRoot.next = NULL;
}
struct PendingNode {
  int pos;
  int value;
  struct PendingNode* next;
};
static struct PendingNode pendingRoot;
static struct PendingNode* pending_words = &pendingRoot;
void reset_pending_words() {
  pending_words = &pendingRoot;
  pendingRoot.next = NULL;
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
  to_mnemonic_symbol("bne", 3);  
  to_mnemonic_symbol("mov", 3);
  to_mnemonic_symbol(".raw", 4);
  to_mnemonic_symbol("ldr", 3);
  to_mnemonic_symbol("ldrb", 4);
  to_mnemonic_symbol("str", 3);
  to_mnemonic_symbol("cmp", 3);
  to_mnemonic_symbol("add", 3);
  to_mnemonic_symbol("sub", 3);
  to_mnemonic_symbol("lsr", 3);
  to_mnemonic_symbol("and", 3);
  to_mnemonic_symbol("bge", 3);
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
  char* p = str;
  if(*str == '\0') return 0;
  if(*str == ':') {
      out_subs->str = str;
      out_subs->len = 1;
      return 1;
  }
  if(is_space(*str)) {
    char* p = str;
    skip_space(str);
    out_subs->str = str;
    out_subs->len = (str - p) - 1;
    return out_subs->len;
  }
  if(*str == '.' || is_alnum(*str)) {    
    while(*str == '.' || is_alnum(*str) || *str == '_') str++;
    out_subs->str = p;
    out_subs->len = str - p;
    return out_subs->len;
  }
  return PARSE_FAIL;
}

int parse_register(char* str, int* out_register) {
  int  pos = 0;
  char* p = str;
  skip_space(str);
  if(*str == 'r') {
    int v = 0;
    int c = *(++str);
    do {
      v = v * 10 + c - '0';
      c = *(++str);
    }while(is_digit(c));
    *out_register = v;
    return str - p;
  } else {
    return PARSE_FAIL;
  }
}

int parse_hex(char* str, int* out_value) {
  int  pos = 0;
  char* p = str;
  skip_space(p);
  int is_negative = 0;
  if(str[pos] == '-') {
    is_negative = 1;
    str++;
  }
  if(strncmp(str, "0x", 2) != 0) return PARSE_FAIL;
  str+=2;
  int v = 0;
  int c = *str;
  while(is_hex(c)) {
    if(is_digit(c)) {
      v = v * 16 + c - '0';
    } else if(is_capital(c)) {
      v = v * 16 + c - 'A' + 10;
    } else {
      v = v * 16 + c - 'a' + 10;
    }
    c = *(++str);
  }
  *out_value = (is_negative)? -v : v;
  return str - p;
}

int parse_immediate(char* str, int* out_value) {
  skip_space(str);
  if(strncmp(str, "#", 1) != 0) return PARSE_FAIL;
  return parse_hex((str + 1), out_value) + 1;
}

int skip_symbol(char* str, int symbol) {
  char* p = str;
  skip_space(str);
  if(*str == symbol) return (str - p) + 1;
  else return PARSE_FAIL;
}

int asm_str(char* str, struct AsmNode* node) {
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

  node->u.word = 0xE5800000  + (rd << 16) + (rn << 12);
  return 1;
}

int asm_cmp(char* str, struct AsmNode* node) {
  int n, rn, imm;
  n = parse_register(str, &rn);
  if(n == PARSE_FAIL) return PARSE_FAIL;
  str += n;
  n = skip_symbol(str, ',');
  if(n == PARSE_FAIL) return PARSE_FAIL;
  str += n;
  skip_space(str);

  n = parse_immediate(str, &imm);
  if(n == PARSE_FAIL) return PARSE_FAIL;
  node->u.word = 0xE3500000 + (rn << 16) + imm;
  node->type = WORD;
  return 1;
}

int asm_add(char* str, struct AsmNode* node) {
  int n, rn, rd, imm;
  n = parse_register(str, &rd);
  if(n == PARSE_FAIL) return PARSE_FAIL;
  str += n;
  n = skip_symbol(str, ',');
  if(n == PARSE_FAIL) return PARSE_FAIL;
  str += n;
  skip_space(str);

  n = parse_register(str, &rn);
  if(n == PARSE_FAIL) return PARSE_FAIL;
  str += n;
  n = skip_symbol(str, ',');
  if(n == PARSE_FAIL) return PARSE_FAIL;
  str += n;
  skip_space(str);

  n = parse_immediate(str, &imm);
  if(n == PARSE_FAIL) return PARSE_FAIL;
  node->u.word = 0xE2800000 + (rn << 16) + (rd << 12) + imm;
  return 1;
}

int asm_sub(char* str, struct AsmNode* node) {
  int n, rn, rd, rm;
  n = parse_register(str, &rn);
  if(n == PARSE_FAIL) return PARSE_FAIL;
  str += n;
  n = skip_symbol(str, ',');
  if(n == PARSE_FAIL) return PARSE_FAIL;
  str += n;
  skip_space(str);

  n = parse_register(str, &rd);
  if(n == PARSE_FAIL) return PARSE_FAIL;
  str += n;
  n = skip_symbol(str, ',');
  if(n == PARSE_FAIL) return PARSE_FAIL;
  str += n;
  skip_space(str);

  int is_immediate = !is_register(*str);
  if(!is_immediate) {
    n = parse_register(str, &rm);
  } else {
    n = parse_immediate(str, &rm);
  }
  if(n == PARSE_FAIL) return PARSE_FAIL;
  node->u.word = 0xE0400000 + (rd << 16) + (rn << 12) + rm + (is_immediate << 25);
  return 1;
}

void pending_add(int addr, int word);
int asm_ldr(char* str, struct AsmNode* node, int addr, int base_word) {
  int n, rd, rn;
  int offset = 0;
  n = parse_register(str, &rn);
  if(n == PARSE_FAIL) return PARSE_FAIL;
  str += n;
  n = skip_symbol(str, ',');
  if(n == PARSE_FAIL) return PARSE_FAIL;
  str += n;
  char* p = str;
  skip_space(str);
  if(*str == '=') {
    rd = 15;
    n = skip_symbol(str, '=');
    if(n == PARSE_FAIL) return PARSE_FAIL;
    str += n;
    if(*str == '0' || *str == '-') {
      int word;
      n = parse_hex(str, &word);
      if(n == PARSE_FAIL) return PARSE_FAIL;
      pending_add(addr, word);
      node->u.word = base_word + (1 << 23) + (rd << 16) + (rn << 12);
      node->type = WORD;
    } else {
      rd = 15;
      struct substring out_subs = {0};
      skip_space(str);
      n = parse_one(str, &out_subs);
      if(n == PARSE_FAIL) return PARSE_FAIL;
      int label_id = to_label_symbol(out_subs.str, out_subs.len);
      symbol_add(addr, label_id);
      node->type = WORD;
      node->u.word = base_word + (1 << 23) + (rd << 16) + (rn << 12);
    }
  } else if(*str == '[') {
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
      skip_space(str);
      n = parse_immediate(str, &offset);
      if(n == PARSE_FAIL) return PARSE_FAIL;
      str += n;
      n = skip_symbol(str, ']');
      if(n == PARSE_FAIL) return PARSE_FAIL;
    }
    node->u.word = base_word + ((offset >= 0) << 23) + (rd << 16) + (rn << 12) + abs(offset);
    node->type = WORD;
  }
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

int asm_raw(char* str, struct AsmNode* node) {
  int n, v;
  skip_space(str);
  if(str[0] == '"') {
    char* out_str;
    n = parse_string(str, &out_str);
    if(n == PARSE_FAIL) return PARSE_FAIL;
    node->type = RAW;
    node->u.str = out_str;
  } else {
    n = parse_hex(str, &v);
    if(n == PARSE_FAIL) return PARSE_FAIL;
    node->type = WORD;
    node->u.word = v;
  }
  return 1;
}

int asm_mov(char* str, struct AsmNode* node) {
  int n, r1, rm;
  n = parse_register(str, &r1);
  if(n == PARSE_FAIL) return PARSE_FAIL;
  str += n;
  n = skip_symbol(str, ',');
  if(n == PARSE_FAIL) return PARSE_FAIL;
  str += n;
  skip_space(str);
  int is_immediate = !is_register(str[0]);
  if(!is_immediate) {
    n = parse_register(str, &rm);
  } else {
    n = parse_immediate(str, &rm);
  }
  if(n == PARSE_FAIL) return PARSE_FAIL;
  node->u.word = 0xE1A00000 + (r1 << 12) + rm + (is_immediate << 25);
  return 1;
}

int asm_lsr(char* str, struct AsmNode* node) {
  int n, rm, rd, rs;
  n = parse_register(str, &rd);
  if(n == PARSE_FAIL) return PARSE_FAIL;
  str += n;
  n = skip_symbol(str, ',');
  if(n == PARSE_FAIL) return PARSE_FAIL;
  str += n;
  skip_space(str);

  n = parse_register(str, &rm);
  if(n == PARSE_FAIL) return PARSE_FAIL;
  str += n;
  n = skip_symbol(str, ',');
  if(n == PARSE_FAIL) return PARSE_FAIL;
  str += n;
  skip_space(str);

  int is_immediate = !is_register(*str);
  if(!is_immediate) {
    n = parse_register(str, &rs);
  } else {
    n = parse_immediate(str, &rs);
  }
  if(n == PARSE_FAIL) return PARSE_FAIL;
  if(is_immediate) {
    node->u.word = 0xE1A00000 + (rd << 12) + (rs << 7) + rm + 0x20;
  } else {
    node->u.word = 0xE1A00000 + (rd << 12) + (rs << 8) + rm + 0x30;   
  }
  return 1;
}

int asm_and(char* str, struct AsmNode* node) {
  int n, rn, rd, imm;
  n = parse_register(str, &rd);
  if(n == PARSE_FAIL) return PARSE_FAIL;
  str += n;
  n = skip_symbol(str, ',');
  if(n == PARSE_FAIL) return PARSE_FAIL;
  str += n;
  skip_space(str);

  n = parse_register(str, &rn);
  if(n == PARSE_FAIL) return PARSE_FAIL;
  str += n;
  n = skip_symbol(str, ',');
  if(n == PARSE_FAIL) return PARSE_FAIL;
  str += n;
  skip_space(str);

  n = parse_immediate(str, &imm);
  if(n == PARSE_FAIL) return PARSE_FAIL;
  node->u.word = 0xE2000000 + (rn << 16) + (rd << 12) + imm;
  return 1;
}

int asm_b(char* str, struct AsmNode* node, int addr, int base_word) {
  int n;
  struct substring out_subs = {0};
  skip_space(str);
  n = parse_one(str, &out_subs);
  if(n == PARSE_FAIL) return n;
  int label_id = to_label_symbol(out_subs.str, out_subs.len);
  symbol_add(addr, label_id);
  node->u.word = base_word;
  return 1;
}

int asm_one(char* str, struct AsmNode* node, int addr) {
  int n;
  struct substring out_subs = {0};
  int r1, rm;
  skip_space(str);
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
    return asm_b(str, node, addr, 0xEA000000);
  case 1:
    return asm_b(str, node, addr, 0x1A000000);
  case 2:
    return asm_mov(str, node);
  case 3:
    return asm_raw(str, node);
  case 4:
    return asm_ldr(str, node, addr, 0xE5100000);
  case 5:
    return asm_ldr(str, node, addr, 0xE5500000);
  case 6:
    return asm_str(str, node);
  case 7:
    return asm_cmp(str, node);
  case 8:
    return asm_add(str, node);
  case 9:
    return asm_sub(str, node);
  case 10:
    return asm_lsr(str, node);
  case 11:
    return asm_and(str, node);
  case 12:
    return asm_b(str, node, addr, 0xAA000000);
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
    printf("0x%08X\n", array[i]);
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
void pending_add(int pos, int value) {
  struct PendingNode* v = malloc(sizeof(struct PendingNode));
  v->pos = pos;
  v->value = value;
  v->next = NULL;
  pending_words->next = v;
  pending_words = v;
}

void resolve_pendings(struct Emitter* emitter) {
    {
    struct PendingNode* p = &pendingRoot;
    while(p->next != NULL) {
      p = p->next;
      int addr = emitter->pos;
      array[emitter->pos++] = p->value;
      int word = array[p->pos];
      int r_addr = (addr - 2 - p->pos) * 4;
      array[p->pos] = word | r_addr;
    }
  }
}
static int is_b(int word) {
  return (((word >> 24) & 0xF) & 0B1010) == 0B1010;
}

void resolve_symbols(struct Emitter* emitter) {
  struct SymbolNode* p = &symbolRoot;
  while(p->next != NULL) {
    p = p->next;
    int addr;
    if(dict_get(p->label_id, &addr) == 1) {
      int word = array[p->pos];
      if(is_b(word)) {
        int r_addr = addr - p->pos - 1;
        array[p->pos] = word | (0XFFFFFF & (0XFFFFFF + r_addr));
      } else {
        int addr2 = emitter->pos++;
        array[addr2] = 0X00010000 | (addr * 4);
        array[p->pos] = word | (addr2 - p->pos - 2) * 4;
      }
    }
  }
}

static void test_ldr_message_label() {
  reset_pending_words();
  reset_unresolve_list();
  reset_symbols();
  reset_dict();
  setup_symbols();
  struct AsmNode node;
  reset_dict();
  int addr = 0;
  struct Emitter emitter = {0};
  assert(asm_one("ldr r1,=message\n", &node, 0) != PARSE_FAIL);
  assert(node.type == WORD);
  assert(node.u.word == 0xE59F1000);
  emit_word(&emitter, node.u.word);
  assert(asm_one("mov r1, r2\n", &node, 1) != PARSE_FAIL);
  emit_word(&emitter, node.u.word);
  emit_word(&emitter, node.u.word);
  emit_word(&emitter, node.u.word);
  assert(asm_one("message:", &node, 4) != PARSE_FAIL);
  assert(asm_one(".raw \"hello, world\\n\"", &node, 4) != PARSE_FAIL);
  char* str = node.u.str;
  int n = strlen(str) / 4;
  int out_word;
  for(int i = 0; i <= n; i++) {
    str_to_word(str, &out_word);
    emit_word(&emitter, out_word);
    addr++;
    str += 4;
  }
  resolve_pendings(&emitter);
  resolve_symbols(&emitter);
  assert(array[0] == 0xE59F1018);
  assert(array[8] == 0x00010010);
}

static void test_ldr_immediate_label() {
  reset_pending_words();
  reset_unresolve_list();
  struct AsmNode node;
  reset_dict();
  setup_symbols();
  int addr = 0;
  struct Emitter emitter = {0};
  assert(asm_one("ldr r1,=0x101f1000\n", &node, 0) != PARSE_FAIL);
  assert(node.type == WORD);
  assert(node.u.word == 0xE59F1000);
  emit_word(&emitter, node.u.word);
  assert(asm_one("mov r1, r2\n", &node, 1) != PARSE_FAIL);
  emit_word(&emitter, node.u.word);
  emit_word(&emitter, node.u.word);
  emit_word(&emitter, node.u.word);
  emit_word(&emitter, node.u.word);
  resolve_pendings(&emitter);
  resolve_symbols(&emitter);
  assert(array[0] == 0xE59F100c);
  assert(array[5] == 0x101f1000);
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
  struct AsmNode node;
  reset_symbols();
  reset_dict();
  setup_symbols();
  int addr = 0;
  struct Emitter emitter = {0};
  emitter.elems = array;
  assert(asm_one("loop:\n", &node, 0) == PARSE_LABEL);
  assert(asm_one("mov r1, r2\n", &node, 0) == 1);
  emit_word(&emitter, node.u.word);
  assert(asm_one("b loop\n", &node, 1) == 1);
  emit_word(&emitter, node.u.word);
  resolve_symbols(&emitter);
  assert(array[1] == 0XEAFFFFFD);
}

static void test_bge() {
  struct AsmNode node;
  reset_symbols();
  reset_dict();
  setup_symbols();
  int addr = 0;
  struct Emitter emitter = {0};
  emitter.elems = array;
  assert(asm_one("loop:\n", &node, 0) == PARSE_LABEL);
  assert(asm_one("mov r1, r2\n", &node, 0) == 1);
  emit_word(&emitter, node.u.word);
  assert(asm_one("bge loop\n", &node, 1) == 1);
  emit_word(&emitter, node.u.word);
  resolve_symbols(&emitter);
  assert(array[1] == 0XAAFFFFFD);
}

static void test_bne() {
  struct AsmNode node;
  reset_symbols();
  reset_dict();
  setup_symbols();
  int addr = 0;
  struct Emitter emitter = {0};
  emitter.elems = array;
  assert(asm_one("loop:\n", &node, 0) == PARSE_LABEL);
  assert(asm_one("mov r1, r2\n", &node, 0) == 1);
  emit_word(&emitter, node.u.word);
  assert(asm_one("bne loop\n", &node, 1) == 1);
  emit_word(&emitter, node.u.word);
  resolve_symbols(&emitter);
  assert(array[1] == 0X1AFFFFFD);
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
  struct AsmNode node;
  reset_symbols();
  reset_dict();
  int addr = 0;
  assert(asm_one("loop:\n", &node, ++addr) == PARSE_LABEL);
  assert(asm_one("sum:\n", &node, ++addr) == PARSE_LABEL);
  assert(asm_one("mov:\n", &node, ++addr) == PARSE_LABEL);
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
  reset_symbols();
  setup_symbols();
  assert(to_mnemonic_symbol("mov", 3) == 2);
  assert(to_mnemonic_symbol("str", 3) == 6);
  assert(to_mnemonic_symbol(".raw", 4) == 3);
  assert(to_mnemonic_symbol("ldr", 3) == 4);
  assert(to_mnemonic_symbol("ldrb", 4) == 5);
  reset_symbols();
}

static void test_str() {
  struct AsmNode node;
  assert(asm_one("str r0, [r1]\n", &node, 0) != PARSE_FAIL);
  assert(node.type == WORD);
  assert(node.u.word == 0xE5810000);
}

static void test_ldr() {
  struct AsmNode node;
  assert(asm_one("ldr r1, [r15, #0x30]\n", &node, 0) != PARSE_FAIL);
  assert(node.type == WORD);
  assert(node.u.word == 0xE59F1030);
  assert(asm_one("ldr r1, [r15, #-0x30]\n", &node, 0) != PARSE_FAIL);
  assert(node.type == WORD);
  assert(node.u.word == 0xE51F1030);
  assert(asm_one("ldr r1, [r15]\n", &node, 0) != PARSE_FAIL);
  assert(node.type == WORD);
  assert(node.u.word == 0xE59F1000);
}

static void test_ldrb() {
  struct AsmNode node;
  assert(asm_one("ldrb r3, [r1]\n", &node, 0) != PARSE_FAIL);
  assert(node.type == WORD);
  assert(node.u.word == 0xE5D13000);
}

static void test_cmp() {
  struct AsmNode node;
  assert(asm_one("cmp r3, #0\n", &node, 0) != PARSE_FAIL);
  assert(node.type == WORD);
  assert(node.u.word == 0xE3530000);
}

static void test_cmp2() {
  struct AsmNode node;
  assert(asm_one("cmp r3, #0xa\n", &node, 0) != PARSE_FAIL);
  assert(node.type == WORD);
  assert(node.u.word == 0xE353000A);
}

static void test_add() {
  struct AsmNode node;
  assert(asm_one("add r1, r1, #0x1\n", &node, 0) != PARSE_FAIL);
  assert(node.type == WORD);
  assert(node.u.word == 0xE2811001);
}

static void test_add2() {
  struct AsmNode node;
  assert(asm_one("add r4, r7, #0x5\n", &node, 0) != PARSE_FAIL);
  assert(node.type == WORD);
  assert(node.u.word == 0xE2874005);
}

static void test_and() {
  struct AsmNode node;
  assert(asm_one("and r1, r2, #0x3\n", &node, 0) != PARSE_FAIL);
  assert(node.type == WORD);
  assert(node.u.word == 0xE2021003);
}

static void test_sub() {
  struct AsmNode node;
  assert(asm_one("sub r1, r2, #0x4\n", &node, 0) != PARSE_FAIL);
  assert(node.type == WORD);
  assert(node.u.word == 0xE2421004);
}

static void test_lsr_reg() {
  struct AsmNode node;
  assert(asm_one("lsr r1, r2, r3\n", &node, 0) != PARSE_FAIL);
  assert(node.type == WORD);
  assert(node.u.word == 0xE1A01332);
}

static void test_lsr_imm() {
  struct AsmNode node;
  assert(asm_one("lsr r3, r5, #0x4\n", &node, 0) != PARSE_FAIL);
  assert(node.type == WORD);
  assert(node.u.word == 0xE1A03225);
}

static void test_raw_hex() {
  struct AsmNode node;
  assert(asm_one(".raw 0x12345678\n", &node, 0) != PARSE_FAIL);
  assert(node.u.word == 0x12345678);
}

static void test_raw_str() {
  struct AsmNode node;
  assert(asm_one(".raw \"test\"\n", &node, 0) != PARSE_FAIL);
  assert(node.type == RAW);
  assert(strcmp(node.u.str, "test") == 0);
}

static void test_str_to_word_long() {
  int out_word;
  char* str = "hello, world\n";
  int n = strlen(str) / 4;
  int expects[4] = {
    0x6c6c6568,
    0x77202c6f,
    0x646c726f,
    0x0000000a
  };
  for(int i = 0; i <= n; i++) {
    str_to_word(str, &out_word);
    assert(out_word == expects[i]);
    str += 4;
  }
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
  struct AsmNode node;
  assert(asm_one("mov r1, r2\n", &node, 0) != PARSE_FAIL);
  assert(node.u.word == 0xE1A01002);
  assert(asm_one("mov r2, #0x68\n", &node, 0) != PARSE_FAIL);
  assert(node.u.word == 0xE3A02068);
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
  test_str_to_word_long();
  test_ldr_immediate_label();
  test_ldr_message_label();
  test_ldrb();
  test_bne();
  test_sub();
  test_lsr_reg();
  test_lsr_imm();
  test_and();
  test_bge();
  test_cmp2();
  test_add2();
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
      struct AsmNode node;
      int result = asm_one(out_char, &node, addr);
      if(result == PARSE_FAIL) {
        fprintf(stderr, "%s: parse failed\n", out_char);
        return PARSE_FAIL;
      }
      if(result != PARSE_LABEL) {
        switch(node.type) {
          case WORD:
          emit_word(&emitter, node.u.word);
          addr++;
          break;
          case RAW: {
            char* str = node.u.str;
            int n = strlen(str) / 4;
            int out_word;
            for(int i = 0; i <= n; i++) {
              str_to_word(str, &out_word);
              emit_word(&emitter, out_word);
              addr++;
              str += 4;
            }
          }
        }
      }
    }
    resolve_pendings(&emitter);
    resolve_symbols(&emitter);
    save_words(&emitter);
    print_words(&emitter);
  }
  return 0;
}