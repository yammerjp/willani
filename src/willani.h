#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


//======================================
// tokenize.c
typedef enum {
  TK_RESERVED,  // Keywords or punctuators
  TK_IDENT,     // Identifers
  TK_NUM,       // Numeric literals
  TK_EOF,       // End-of-file markers
} TokenKind;

typedef struct Token Token;
struct Token {
  TokenKind kind;
  Token *next;
  char *location;
  int length;
};

bool is_number_token(Token *token);
bool is_identifer_token(Token *token);
bool is_eof_token(Token *token);
bool equal(Token *token, char *str);
Token *tokenize(char *p);


//======================================
// parse.c
typedef enum {
  ND_ADD,         // +
  ND_SUB,         // -
  ND_MUL,         // *
  ND_DIV,         // /
  ND_EQ,          // ==
  ND_NE,          // !=
  ND_LT,          // <
  ND_LE,          // <=
  ND_ASSIGN,      // =
  ND_VAR,         // Variable
  ND_NUM,         // Integer
  ND_FUNC_CALL,   // Function call
  ND_ADDR,        // & ...
  ND_DEREF,       // * ...
  ND_BLOCK,       // { ... }
  ND_IF,          // if
  ND_WHILE,       // while
  ND_FOR,         // for
  ND_RETURN,      // return
  ND_EXPR_STMT,   // ... ;
  ND_DECLARE_VAR, // int ... ;
} NodeKind;


typedef struct LVar LVar;
struct LVar {
  LVar *next;
  char *name;
  int length;
  int offset;
};

typedef struct Node Node;

typedef struct FuncCall FuncCall;
struct FuncCall {
  char *name;
  int length;
  Node *args;
};

struct Node {
  NodeKind kind;
  Node *next;
  Node *left;
  Node *right;
  long value;       // Used if kind == ND_NUM
  LVar *lvar;       // Used if kind == ND_VAR
  Node *cond;       // Used if kind is ND_IF or ND_WHILE or ND_FOR
  Node *then;       // Used if kind is ND_IF or ND_WHILE or ND_FOR
  Node *els;        // Used if kind == ND_IF
  Node *init;       // Used if kind == ND_FOR
  Node *increment;  // Used id kind == ND_FOR
  Node *body;       // Used if kind == ND_BLOCK
  FuncCall *fncl;   // Used if kind == ND_FUNC_CALL
};

typedef struct Function Function;
struct Function {
  Node *node;
  LVar *lvar;
  Function *next;
  char *name;
  int namelen;
  LVar *args;
};

Function *program(Token *token);


//======================================
// parse_log.c
void print_node(FILE *file, Node *node);
void parse_log(Function *function);


//======================================
// gen.c
void code_generate(Function *func);


//======================================
// error.c
void error_at(Token *prevtoken, char *fmt, ...);
void error(char *fmt, ...);


//======================================
// main.c
extern char *user_input;
