#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


//======================================
// tokenize.c
typedef enum {
  TK_RESERVED, // Keywords or punctuators
  TK_IDENT, // Identifers
  TK_NUM, // Numeric literals
  TK_EOF, // End-of-file markers
} TokenKind;

typedef struct Token Token;
struct Token {
  TokenKind kind;
  Token *next;
  long value; // If kind is TK_NUM, its value.
  char *location;
  int length;
};

bool is_number_token(Token *token);
bool is_identifer_token(Token *token);
bool equal(Token *token, char *str);
long get_number(Token *token);
char get_identifer(Token *token);
Token *tokenize(char *p);


//======================================
// parse.c
typedef enum {
  ND_ADD, // +
  ND_SUB, // -
  ND_MUL, // *
  ND_DIV, // /
  ND_EQ,  // ==
  ND_NE,  // !=
  ND_LT,  // <
  ND_LE,  // <=
  ND_ASSIGN,  // =
  ND_VAR,  // Variable
  ND_NUM, // Integer
} NodeKind;

typedef struct Node Node;

struct Node {
  NodeKind kind;
  Node *next;
  Node *left;
  Node *right;
  long value; // Used if kind == ND_NUM
  char name; // Used if kind == ND_NUM
};

Node *program(Token **rest, Token *token);


//======================================
// parse_log.c
void parse_log(Node* head);
char *node_kind_str(Node *node);


//======================================
// gen.c
void code_generate(Node *node);


//======================================
// error.c
void error_at(Token *prevtoken, char *fmt, ...);
void error(char *fmt, ...);


//======================================
// main.c
extern char *user_input;
