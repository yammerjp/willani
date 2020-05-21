#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
  TK_RESERVED, // Keywords or punctuators
  TK_NUM, // Numeric literals
  TK_EOF, // End-of-file markers
} TokenKind;

typedef struct Token Token;

struct Token {
  TokenKind kind;
  Token *next;
  long value; // If kind is TK_NUM, its value
  char *location;
  int length;
};

Token *token;

static void error(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

static bool equal(Token *token, char *str) {
  return strlen(str) == token->length &&
          !strncmp(token->location, str, token->length);
}

static long get_number(Token *token) {
  if (token->kind != TK_NUM)
    error("expected a number");
  return token->value;
}

// 新しいトークンを作成しcurにつなげる
static Token *new_token(TokenKind kind, Token *current, char *location, int length) {
  Token *newtoken = calloc(1, sizeof(Token));
  newtoken->kind = kind;
  newtoken->location = location;
  newtoken->length = length;
  current->next = newtoken;
  return newtoken;
}

Token *tokenize(char *p) {
  Token head = {};
  Token *current = &head;

  while (*p) {
    if (isspace(*p)) {
      p++;
      continue;
    }

    if (isdigit(*p)) {
      current = new_token(TK_NUM, current, p, 0); // length is still unknown
      char *p_old = p;
      current->value = strtol(p, &p, 10); // with update p
      current->length = p - p_old;
      continue;
    }

    if (*p == '+' || *p == '-') {
      current = new_token(TK_RESERVED, current, p++, 1); //with update p
      continue;
    }

    error("Invalid token");
  }

  new_token(TK_EOF, current, p, 0);
  return head.next;
}

int main(int argc, char **argv) {
  if (argc != 2)
    error("%s: invalid number of arguments.\n", argv[0]);

  token = tokenize(argv[1]);

  printf(".intel_syntax noprefix\n");
  printf(".globl main\n");
  printf("main:\n");

  // The first toke must be a number.
  printf("  mov rax, %ld\n", get_number(token));
  token = token->next;

  while (token->kind != TK_EOF) {
    if (equal(token, "+")) {
      token = token->next;
      printf("  add rax, %ld\n", get_number(token));
      token = token->next;
      continue;
    }

    if (equal(token, "-")) {
      token = token->next;
      printf("  sub rax, %ld\n", get_number(token));
      token = token->next;
      continue;
    }

    error("unexpected token.");
  }

  printf("  ret\n");
  return 0;
}
