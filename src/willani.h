#ifndef INCLUDED_willani_h_
#define INCLUDED_willani_h_

#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Type Type;

//======================================
// parse/strings.c
typedef struct String String;
struct String {
  int id;
  char *p;
  int length;
  String *next;
};
extern String *strings;

typedef struct Member Member;
struct Member {
  Member *next;
  Type *type;
  char *name;
  int namelen;
  int offset;  // offset from the memory address of parent
};

//======================================
// tokenize.c

typedef enum {
  TK_RESERVED,  // Keywords or punctuators
  TK_IDENT,     // Identifers
  TK_STRING,    // "..."
  TK_CHAR,      // '.'
  TK_STRUCT,    // struct
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
bool is_char_token(Token *token);
bool is_string_token(Token *token);
bool is_eof_token(Token *token);
bool equal(Token *token, char *str);
Token *tokenize(char *p);

char *get_line_head(char *head);
char *get_line_end(char *end);
int get_line_number(char *line_head);


//======================================
// type.c
typedef enum {
  TYPE_LONG,
  TYPE_INT,
  TYPE_CHAR,
  TYPE_BOOL,
  TYPE_PTR,
  TYPE_ARRAY,
  TYPE_STRUCT,
} TypeKind;

struct Type {
  TypeKind kind;
  int size;
  Type *base;       // Used if kind is TYPE_PTR
  int array_length; // Used if kind is TYPE_ARRAY
  Member *members;   // Used if kind is TYPE_STRUCT
};

Type *new_type_long();
Type *new_type_int();
Type *new_type_char();
Type *new_type_bool();
Type *new_type_pointer(Type *parent);
Type *new_type_array(Type *parent, int length);
Type *new_type_struct(int size, Member *members);
Member *new_member(char *name, int namelen, Type *type, int offset);
Member *find_member(Type *type, char *name, int namelen);
Type *type_conversion(Type *left, Type *right);
bool cmp_type(Type *t1, Type *t2);
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
  bool is_typedef;
  bool referable;
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
  ND_STRING,      // " ... "
  ND_FUNC_CALL,   // Function call
  ND_ADDR,        // & ...
  ND_DEREF,       // * ...
  ND_MEMBER,      // struct member access
  ND_BLOCK,       // { ... }
  ND_IF,          // if
  ND_WHILE,       // while
  ND_FOR,         // for
  ND_RETURN,      // return
  ND_EXPR_STMT,   // ... ;
  ND_STMT_EXPR,   // ({ ...; ... }) GNU statement expression extention
} NodeKind;

typedef struct Node Node;

struct Node {
  NodeKind kind;
  Token *token;     // Representative token (FOR DEBUG!!!)

  Type *type;       // Used if node is expression
  long value;       // Used if kind == ND_NUM
  String *string;   // Used if kind == ND_STRING
  Var *var;         // Used if kind is ND_GVAR or ND_LVAR
  Member *member;   // Used if kind is ND_MEMBER (struct member access)

  char *func_name;  // Used if kind == ND_FUNC_CALL
  int func_namelen; // Used if kind == ND_FUNC_CALL
  Node *func_args;  // Used if kind == ND_FUNC_CALL

  Node *next;
  Node *left;
  Node *right;
  Node *cond;       // Used if kind is ND_IF or ND_WHILE or ND_FOR // expr
  Node *then;       // Used if kind is ND_IF or ND_WHILE or ND_FOR
  Node *els;        // Used if kind == ND_IF
  Node *init;       // Used if kind == ND_FOR  // stmt
  Node *increment;  // Used id kind == ND_FOR  // expression statement
  Node *body;       // Used if kind is ND_BLOCK or ND_STMT_EXPR
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
  bool definition;
};

void *program(Token *token);


//======================================
// parse/var.c
extern Var *gvars;
extern Var *lvars;
extern Var *outer_scope_lvars;


//======================================
// parse/log.c
void print_node(FILE *file, Node *node);



//======================================
// parse/new_node.c
Type *type_suffix(Token **rest, Token *token, Type *ancestor);


//======================================
// gen.c
void code_generate();


//======================================
// error.c
void error_at(char *location, char *msg);
void error(char *fmt, ...);


//======================================
// main.c
extern char *filename;
extern char *user_input;
extern Function *functions;


//======================================
// read_file.c
char *read_file(char *path);

#endif
