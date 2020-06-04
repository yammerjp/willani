#ifndef INCLUDED_parse_h_
#define INCLUDED_parse_h_

#include "../willani.h"

Function *function(Token **rest, Token *token, Type *return_type, char *name, int namelen);
Node *stmt(Token **rest, Token *token, Var **lvarsp);
Node *if_stmt(Token **rest, Token *token, Var **lvarsp);
Node *while_stmt(Token **rest, Token *token, Var **lvarsp);
Node *for_stmt(Token **rest, Token *token, Var **lvarsp);
Node *block_stmt(Token **rest, Token *token, Var **lvarsp);
Node *expr_stmt(Token **rest, Token *token, Var **lvarsp);
Node *return_stmt(Token **rest, Token *token, Var **lvarsp);
Node *declare_lvar_stmt(Token **rest, Token *token, Var **lvarsp, Type *type);
Type *type_suffix(Token **rest, Token *token, Type *ancestor);
Node *expr(Token **rest, Token *token, Var **lvarsp);
Node *assign(Token **rest, Token *token, Var **lvarsp);
Node *equality(Token **rest, Token *token, Var **lvarsp);
Node *relational(Token **rest, Token *token, Var **lvarsp);
Node *add(Token **rest, Token *token, Var **lvarsp);
Node *mul(Token **rest, Token *token, Var **lvarsp);
Node *unary(Token **rest, Token *token, Var **lvarsp);
Node *sizeofunary(Token **rest, Token *token, Var **lvarsp);
Node *primary(Token **rest, Token *token, Var **lvarsp);

typedef struct ArrayIndexes ArrayIndexes;
struct ArrayIndexes {
  int index;
  ArrayIndexes *parent;
};

Node *init_lvar_stmts(Token **rest, Token *token, Var **lvarsp, ArrayIndexes *descendant);

#endif
