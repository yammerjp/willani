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
// str_to_l.c
long str_to_l(char *p, int length);

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
  bool is_static;
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
bool same_type(Type *t1, Type *t2);
bool is_ptr_or_arr(Type *type);
extern const int type_size_pointer;


//======================================
// parse/main.c
// identifer of variable and typedef
typedef struct Var Var;
struct Var {
  Type *type;
  Var *next;
  char *name;
  int namelen;
  int offset;
  bool is_typedef;
  bool is_global;
};

typedef enum {
  // single operator
  ND_EXPR_VAR,          // Variable
  ND_EXPR_NUM,          // Integer
  ND_EXPR_STRING,       // " ... "
  ND_EXPR_FUNC_CALL,    // Function call
  ND_EXPR_ADDR,         // & ...
  ND_EXPR_DEREF,        // * ...
  ND_EXPR_NOT,          // ! ...
  ND_EXPR_BIT_NOT,      // ~ ...
  ND_EXPR_PRE_INC,      // ++ ...
  ND_EXPR_PRE_DEC,      // -- ...
  // dual operator
  ND_EXPR_ASSIGN,       // =
  ND_EXPR_ASSIGN_ADD,   // +=
  ND_EXPR_ASSIGN_PTR_ADD, // (pointer) +=
  ND_EXPR_ASSIGN_SUB,   // -=
  ND_EXPR_ASSIGN_PTR_SUB, // (pointer) -=
  ND_EXPR_ASSIGN_MUL,   // *=
  ND_EXPR_ASSIGN_DIV,   // /=
  ND_EXPR_ASSIGN_MOD,   // %=
  ND_EXPR_ADD,          // +
  ND_EXPR_PTR_ADD,      // (pointer) + (int)
  ND_EXPR_SUB,          // -
  ND_EXPR_PTR_SUB,      // (pointer) - (int)
  ND_EXPR_PTR_DIFF,     // (pointer) - (pointer)
  ND_EXPR_MUL,          // *
  ND_EXPR_DIV,          // /
  ND_EXPR_MOD,          // %
  ND_EXPR_EQ,           // ==
  ND_EXPR_NE,           // !=
  ND_EXPR_LESS_THAN,    // <
  ND_EXPR_LESS_EQ,      // <=
  ND_EXPR_MEMBER,       // . struct member access
  ND_EXPR_COMMA,        // ... , ...
  // multiple operator
  ND_EXPR_WITH_STMTS,   // ({ ...; ... }) GNU statement expression extention
  // statement
  ND_STMT_BLOCK,        // { ... }
  ND_STMT_IF,           // if
  ND_STMT_SWITCH,       // switch { ... }
  ND_STMT_WHILE,        // while
  ND_STMT_FOR,          // for
  ND_STMT_RETURN,       // return
  ND_STMT_WITH_EXPR,    // ... ;
  ND_STMT_CONTINUE,     // continue;
  ND_STMT_BREAK,        // break;
  ND_LABEL_CASE,        // case expr: in switch statement
  ND_LABEL_DEFAULT,     // default: in switch statement
} NodeKind;

typedef struct Node Node;

struct Node {
  NodeKind kind;
  Token *token;     // Representative token (FOR DEBUG!!!)

  Type *type;       // Used if kind is  ND_EXPR_*
  long value;       // Used if kind is  ND_EXPR_NUM
  String *string;   // Used if kind is  ND_EXPR_STRING
  Var *var;         // Used if kind is  ND_EXPR_VAR
  Member *member;   // Used if kind is  ND_EXPR_MEMBER (struct member access)

  char *func_name;  // Used if kind is  ND_EXPR_FUNC_CALL
  int func_namelen; // Used if kind is  ND_EXPR_FUNC_CALL
  Node *func_args;  // Used if kind is  ND_EXPR_FUNC_CALL

  Node *next;
  Node *left;
  Node *right;
  Node *cond;       // Used if kind is  ND_STMT_IF, ND_STMT_WHILE, ND_STMT_FOR  // expr
  Node *then;       // Used if kind is  ND_STMT_IF, ND_STMT_WHILE, ND_STMT_FOR  // stmt
  Node *els;        // Used if kind is  ND_STMT_IF                              // stmt
  Node *init;       // Used if kind is  ND_STMT_FOR                             // stmt
  Node *increment;  // Used id kind is  ND_STMT_FOR                             // expression statement
  Node *body;       // Used if kind is  ND_STMT_BLOCK, ND_EXPR_WITH_STMTS
  Node *cases;      // Used if kind is  ND_STMT_SWITCH
  int case_num;     // Used if kind is  ND_LABEL_CASE
  bool have_default;// Used if kind is  ND_STMT_SWITCH
};

typedef struct Function Function;
struct Function {
  Node *node;
  int var_byte;
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
