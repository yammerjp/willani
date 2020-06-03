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
  TYPE_INT,
  TYPE_PTR,
  TYPE_ARRAY,
} TypeKind;

typedef struct Type Type;
struct Type {
  TypeKind kind;
  Type *ptr_to;  // Used if kind is TYPE_PTR
  size_t array_size;
};

Type *new_type_int();
Type *new_type_pointer(Type *parent);
Type *new_type_array(Type *parent, size_t array_size);
int type_size(Type *type);
Type *read_type_tokens(Token **rest, Token *token);
Type *read_type_tokens_with_pars(Token **rest, Token *token);

//======================================
// parse.c
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
  Type *type;        // Used if node is expression
  long value;       // Used if kind == ND_NUM
  Var *var;       // Used if kind is ND_GVAR or ND_LVAR
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
// parse_new_node.c
Var *find_var(char *name, int length, Var *vars);
void *new_var(Type *type, char *name, int length, Var **varsp);
void *new_gvar(Type *type, char *name, int length);
Node *new_node_op2(NodeKind kind, Node *left, Node *right);
Node *new_node_num(long value);
Node *new_node_var(char *name, int length, Var *lvars);
Node *new_node_declare_lvar(Type *type, char *name, int length, Var **lvarsp);
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
extern Var *gvars;

//======================================
// type.c
void add_type(Node *node);
