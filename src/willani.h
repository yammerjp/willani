#ifndef INCLUDED_willani_h_
#define INCLUDED_willani_h_

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
// type.c
typedef enum {
  TYPE_LONG,
  TYPE_INT,
  TYPE_CHAR,
  TYPE_PTR,
  TYPE_ARRAY,
} TypeKind;

typedef struct Type Type;
struct Type {
  TypeKind kind;
  Type *ptr_to;     // Used if kind is TYPE_PTR
  int array_length; // Used if kind is TYPE_ARRAY
};

Type *new_type_long();
Type *new_type_int();
Type *new_type_char();
Type *new_type_pointer(Type *parent);
Type *new_type_array(Type *parent, int length);
int type_size(Type *type);
Type *read_type_tokens(Token **rest, Token *token);
Type *read_type_tokens_with_pars(Token **rest, Token *token);
Type *type_conversion(Type *left, Type *right);
extern const int type_size_pointer;

//======================================
// parse/main.c
typedef struct Var Var;
struct Var {
  Type *type;
  Var *next;
  char *name;
  int length;
  int offset;
};

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
  ND_GVAR,         // Global Variable
  ND_LVAR,         // Variable
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
} NodeKind;

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
  Type *type;       // Used if node is expression
  long value;       // Used if kind == ND_NUM
  Var *var;         // Used if kind is ND_GVAR or ND_LVAR
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
  Var *var;
  Function *next;
  char *name;
  int namelen;
  Var *args;
  int argc;
  Type *type;
};

Function *program(Token *token);


//======================================
// parse/var.c
extern Var *gvars;


//======================================
// parse/log.c
void print_node(FILE *file, Node *node);


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

#endif
