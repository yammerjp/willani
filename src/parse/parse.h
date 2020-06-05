#ifndef INCLUDED_parse_h_
#define INCLUDED_parse_h_

#include "../willani.h"


//======================================
// var.c
Var *find_var(char *name, int length, Var *vars);
void *new_var(Type *type, char *name, int length, Var **varsp);
void *new_gvar(Type *type, char *name, int length);


//======================================
// new_node.c
Type *type_suffix(Token **rest, Token *token, Type *ancestor);

Node *new_node_add(Node *left, Node *right);
Node *new_node_sub(Node *left, Node *right);
Node *new_node_mul(Node *left, Node *right);
Node *new_node_div(Node *left, Node *right);
Node *new_node_equal(Node *left, Node *right);
Node *new_node_not_equal(Node *left, Node *right);
Node *new_node_less_than(Node *left, Node *right);
Node *new_node_less_equal(Node *left, Node *right);

Node *new_node_num(long value);
Node *new_node_var(char *name, int length, Var *lvars);
Node *new_node_return(Node *left);
Node *new_node_if(Node *cond, Node *then, Node *els);
Node *new_node_while(Node *cond, Node *then);
Node *new_node_for(Node *init, Node *cond, Node* increment, Node *then);
Node *new_node_block(Node *body);
Node *new_node_func_call(char *name, int len, Node *args);
Node *new_node_expr_stmt(Node *stmt_node);
Node *new_node_addr(Node *unary_node);
Node *new_node_deref(Node *unary_node);
Node *new_node_assign(Node *left, Node *right);


//======================================
// log.c
void print_node(FILE *file, Node *node);
void parse_log();


//======================================
// stmt.c
Node *stmt(Token **rest, Token *token, Var **lvarsp);
Node *if_stmt(Token **rest, Token *token, Var **lvarsp);
Node *while_stmt(Token **rest, Token *token, Var **lvarsp);
Node *for_stmt(Token **rest, Token *token, Var **lvarsp);
Node *block_stmt(Token **rest, Token *token, Var **lvarsp);
Node *expr_stmt(Token **rest, Token *token, Var **lvarsp);
Node *return_stmt(Token **rest, Token *token, Var **lvarsp);
Node *declare_lvar_stmt(Token **rest, Token *token, Var **lvarsp, Type *type);
Type *type_suffix(Token **rest, Token *token, Type *ancestor);


//======================================
// expr.c
Node *expr(Token **rest, Token *token, Var **lvarsp);
Node *assign(Token **rest, Token *token, Var **lvarsp);
Node *equality(Token **rest, Token *token, Var **lvarsp);
Node *relational(Token **rest, Token *token, Var **lvarsp);
Node *add(Token **rest, Token *token, Var **lvarsp);
Node *mul(Token **rest, Token *token, Var **lvarsp);
Node *unary(Token **rest, Token *token, Var **lvarsp);
Node *sizeofunary(Token **rest, Token *token, Var **lvarsp);
Node *primary(Token **rest, Token *token, Var **lvarsp);
Node *primary_identifer(Token **rest, Token *token, Var **lvarsp);


//======================================
// var_init.c
typedef struct ArrayIndexes ArrayIndexes;
struct ArrayIndexes {
  int index;
  ArrayIndexes *parent;
};

Node *init_lvar_stmts(Token **rest, Token *token, Var **lvarsp, ArrayIndexes *descendant);


//======================================
// function.c
Function *find_function(char *name, int namelen);
void add_function(Function *func);
bool cmp_function(Function *f1, Function *f2);
Function *function_definition(Token **rest, Token *token, Type *return_type, char *name, int namelen);

#endif
