#include "willani.h"

Type *type_int;

Type *new_type_int() {
  Type *type = calloc(1, sizeof(Type));
  type->kind = TYPE_INT;
  return type;
}

int type_size(Type *type) {
  switch (type->kind) {
    case TYPE_INT:
      return 8;
  }
}

Type *read_type_tokens(Token **rest, Token *token) {
  if(!equal(token, "int")) {
    return NULL;
  }
  token = token->next;

  *rest = token;

  return type_int;
}

// Add Type to all expression node after parsing
void add_type(Node *node) {
  if (!node || node->type) {
    return;
  }
  add_type(node->left);
  add_type(node->right);
  add_type(node->cond);
  add_type(node->then);
  add_type(node->els);
  add_type(node->init);
  add_type(node->increment);
  add_type(node->body);
  add_type(node->next);

  switch (node->kind) {
  case ND_LVAR:
    node->type = node->lvar->type;
    return;
  case ND_ADD:
  case ND_SUB:
  case ND_MUL:
  case ND_DIV:
  case ND_ASSIGN:
    node->type = node->left->type;
    return;
  case ND_EQ:
  case ND_NE:
  case ND_LT:
  case ND_LE:
  case ND_NUM:
    node->type = type_int;
    return;
  case ND_FUNC_CALL:
    // TODO: recognize calling function's type
    node->type = type_int;
    return;
  case ND_ADDR:
  case ND_DEREF:
    node->type = type_int;
    return;

  // statement
  case ND_BLOCK:
  case ND_IF:
  case ND_WHILE:
  case ND_FOR:
  case ND_RETURN:
  case ND_EXPR_STMT:
  case ND_DECLARE_LVAR:
    return;
  }
}
