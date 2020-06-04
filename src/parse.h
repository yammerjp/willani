#ifndef INCLUDED_parse_h_
#define INCLUDED_parse_h_

#include "willani.h"

static Function *function(Token **rest, Token *token, Type *return_type, char *name, int namelen);
static Node *stmt(Token **rest, Token *token, Var **lvarsp);
static Node *if_stmt(Token **rest, Token *token, Var **lvarsp);
static Node *while_stmt(Token **rest, Token *token, Var **lvarsp);
static Node *for_stmt(Token **rest, Token *token, Var **lvarsp);
static Node *block_stmt(Token **rest, Token *token, Var **lvarsp);
static Node *expr_stmt(Token **rest, Token *token, Var **lvarsp);
static Node *return_stmt(Token **rest, Token *token, Var **lvarsp);
static Node *declare_lvar_stmt(Token **rest, Token *token, Var **lvarsp, Type *type);
static Type *type_suffix(Token **rest, Token *token, Type *ancestor);
static Node *expr(Token **rest, Token *token, Var **lvarsp);
static Node *assign(Token **rest, Token *token, Var **lvarsp);
static Node *equality(Token **rest, Token *token, Var **lvarsp);
static Node *relational(Token **rest, Token *token, Var **lvarsp);
static Node *add(Token **rest, Token *token, Var **lvarsp);
static Node *mul(Token **rest, Token *token, Var **lvarsp);
static Node *unary(Token **rest, Token *token, Var **lvarsp);
static Node *sizeofunary(Token **rest, Token *token, Var **lvarsp);
static Node *primary(Token **rest, Token *token, Var **lvarsp);

typedef struct ArrayIndexs ArrayIndexs;
struct ArrayIndexs {
  int index;
  ArrayIndexs *parent;
};

static ArrayIndexs *add_descendant(ArrayIndexs *now_descendant, int index);
static Node *new_node_assign_array_cell(Var *var, ArrayIndexs *indexs, Node *right);
static Node *init_lvar_stmt(Token **rest, Token *token, Var **lvarsp, ArrayIndexs *descandant);

#endif
