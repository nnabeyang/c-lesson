#include <assert.h>
#include <string.h>
#include "common.h"
#include <stdio.h>
#include <stdlib.h>
struct substring {
  char* str;
  int len;
};
struct Emitter {
  int* elems;
  int pos;
};
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
  if(is_alnum(str[pos])) {
    while(is_alnum(str[pos]) || str[pos] == '_') pos++;    
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
    int c = str[pos];
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

int parse_immediate(char* str, int* out_value) {
  int  pos = 0;
  while(is_space(str[pos])) pos++;
  if(strncmp(&str[pos], "#0x", 3) != 0) return PARSE_FAIL;
  pos += 3;
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
  *out_value = v;
  return pos;
}
int skip_comma(char* str) {
  if(str[0] == ',') return 1;
  else return PARSE_FAIL;
}
static void test_parse_register() {
  cl_getline_set_str("mov r1, r2");
  char *buf = (char*)malloc(sizeof(char) * 80);
  cl_getline(&buf);
  int r1, r2;
  buf += 4;
  assert(parse_register(buf, &r1) == 2);
  assert(r1 == 1);
  buf += 2;
  assert(skip_comma(buf) == 1);
  buf += 1;
  assert(parse_register(buf, &r2) == 2);
  assert(r2 == 2);
}
int asm_one(char* str) {
  int n;
  struct substring out_subs = {0};
  int r1, r2;
  n = parse_one(str, &out_subs);
  if(n == PARSE_FAIL) return PARSE_FAIL;
  str += n;
  n = parse_register(str, &r1);
  if(n == PARSE_FAIL) return PARSE_FAIL;
  str += n;
  n = skip_comma(str);
  if(n == PARSE_FAIL) return PARSE_FAIL;
  n = parse_register(str, &r2);
  if(n == PARSE_FAIL) return PARSE_FAIL;
  if(strncmp(out_subs.str, "mov", 3) != 0) return PARSE_FAIL;
  return 0xE1A00000 + (r1 << 12) + r2;  
}

static int array[MAX_WORDS];

void emit_word(struct Emitter* emitter, int oneword) {
  array[emitter->pos++] = oneword;
}

static void test_parse_immediate() {
  int v;
  parse_immediate(" #0x68", &v);
  assert(v == 0x68);
}

static void test_asm_one() {
  cl_getline_set_str("mov  r1, r2");
  char *buf = (char*)malloc(sizeof(char) * 80);
  cl_getline(&buf);
  assert(asm_one(buf) == 0xE1A01002);
}

static void test_parse_one() {
  cl_getline_set_str("mov  r1, r2");
  char *buf = (char*)malloc(sizeof(char) * 80);
  cl_getline(&buf);
  struct substring out_subs;
  int n;
  n = parse_one(buf, &out_subs);
  assert(n == 3);
  assert_strn(out_subs.str, "mov", 3);
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

int main() {
  test_getline();
  test_parse_one();
  test_parse_immediate();
}