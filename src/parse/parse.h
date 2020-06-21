#ifndef INCLUDED_parse_h_
#define INCLUDED_parse_h_

#include "../willani.h"


//======================================
// strings.c
char get_escape_char(char c);
String *new_string(char *p, int length);


//======================================
// var.c
Var *find_var(char *name, int length, Var *head, Var *ignore);
void *new_var(Type *type, char *name, int length, Var **varsp);
void *new_gvar(Type *type, char *name, int length);


//======================================
// new_node.c

Node *new_node_add(Node *left, Node *right, Token *token);
Node *new_node_sub(Node *left, Node *right, Token *token);
Node *new_node_mul(Node *left, Node *right, Token *token);
Node *new_node_div(Node *left, Node *right, Token *token);
Node *new_node_equal(Node *left, Node *right, Token *token);
Node *new_node_not_equal(Node *left, Node *right, Token *token);
Node *new_node_less_than(Node *left, Node *right, Token *token);
Node *new_node_less_equal(Node *left, Node *right, Token *token);
Node *new_node_num(long value, Token *token);
Node *new_node_string(Token *token);
Node *new_node_var(char *name, int length, Token *token);
Node *new_node_member(Node *parent, char *name, int namelen,  Token *token);
Node *new_node_return(Node *left, Token *token);
Node *new_node_if(Node *cond, Node *then, Node *els, Token *token);
Node *new_node_while(Node *cond, Node *then, Token *token);
Node *new_node_for(Node *init, Node *cond, Node* increment, Node *then, Token *token);
Node *new_node_block(Node *body, Token *token);
Node *new_node_func_call(char *name, int len, Node *args, Token *token);
Node *new_node_expr_stmt(Node *expr_node, Token *token);
Node *new_node_addr(Node *unary_node, Token *token);
Node *new_node_deref(Node *unary_node, Token *token);
Node *new_node_assign(Node *left, Node *right, Token *token);

//======================================
// log.c
void print_node(FILE *file, Node *node);
void parse_log();


//======================================
// stmt.c
Node *stmt(Token **rest, Token *token, Var *outer_scope_lvars);
Node *stmt_without_declaration(Token **rest, Token *token, Var *outer_scope_lvars);
Node *if_stmt(Token **rest, Token *token);
Node *while_stmt(Token **rest, Token *token);
Node *for_stmt(Token **rest, Token *token);
Node *block_stmt(Token **rest, Token *token, Var *outer_scope_lvars);
Node *expr_stmt(Token **rest, Token *token);
Node *return_stmt(Token **rest, Token *token);
Node *declare_lvar_stmt(Token **rest, Token *token, Type *type, Var *outer_scope_lvars);
Type *type_suffix(Token **rest, Token *token, Type *ancestor);


//======================================
// expr.c
Node *expr(Token **rest, Token *token);
Node *assign(Token **rest, Token *token);
Node *equality(Token **rest, Token *token);
Node *relational(Token **rest, Token *token);
Node *add(Token **rest, Token *token);
Node *mul(Token **rest, Token *token);
Node *unary(Token **rest, Token *token);
Node *sizeofunary(Token **rest, Token *token);
Node *primary(Token **rest, Token *token);
Node *primary_identifer(Token **rest, Token *token);
Node *stmt_expr(Token **rest, Token *token);


//======================================
// var_init.c
typedef struct ArrayIndexes ArrayIndexes;
struct ArrayIndexes {
  int index;
  ArrayIndexes *parent;
};

Node *init_lvar_stmts(Token **rest, Token *token, ArrayIndexes *descendant);


//======================================
// function.c
Function *find_function(char *name, int namelen);
void add_function(Function *func);
bool cmp_function(Function *f1, Function *f2);
Function *function_definition(Token **rest, Token *token, Type *return_type, char *name, int namelen);


//======================================
// read_type.c
Type *read_new_type_struct(Token **rest, Token *token);
Type *read_type(Token **rest, Token *token);
Member *read_member(Token **rest, Token *token, int offset);


#endif
