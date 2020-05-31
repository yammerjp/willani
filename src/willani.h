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

typedef struct LVar LVar;
struct LVar {
  LVar *next;
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
  ND_DECLARE_LVAR, // int ... ;
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
  long value;       // Used if kind == ND_NUM
  LVar *lvar;       // Used if kind == ND_LVAR
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
  int argc;
};

Function *program(Token *token);


//======================================
// parse_new_node.c
LVar *find_lvar(char *name, int length, LVar *lvars);
void *new_lvar(char *name, int length, LVar **lvarsp);
Node *new_node_op2(NodeKind kind, Node *left, Node *right);
Node *new_node_num(long value);
Node *new_node_lvar(char *name, int length, LVar *lvars);
Node *new_node_declare_lvar(char *name, int length, LVar **lvarsp);
Node *new_node_return(Node *left);
Node *new_node_if(Node *cond, Node *then, Node *els);
Node *new_node_while(Node *cond, Node *then);
Node *new_node_for(Node *init, Node *cond, Node* increment, Node *then);
Node *new_node_block(Node *body);
Node *new_node_func_call(char *name, int len, Node *args);
Node *new_node_expr_stmt(Node *stmt_node);
Node *new_node_addr(Node *unary_node);
Node *new_node_deref(Node *unary_node);


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
