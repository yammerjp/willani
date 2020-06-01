#include "willani.h"

Type *type_int;

Type *new_type_int() {
  Type *type = calloc(1, sizeof(Type));
  type->kind = TYPE_INT;
  return type;
}

Type *new_type_pointer(Type *parent) {
  Type *type = calloc(1, sizeof(Type));
  type->kind = TYPE_PTR;
  type->ptr_to = parent;
  return type;
}

int type_size(Type *type) {
  switch (type->kind) {
    case TYPE_INT:
      return 8;
    case TYPE_PTR:
      return 8;
    default:
      error("unknown type size");
  }
}

Type *read_type_tokens(Token **rest, Token *token) {
  Type *type;
  if(equal(token, "int")) {
    type = type_int;
  } else {
    return NULL;
  }
  token = token->next;

  while (equal(token, "*")) {
    type = new_type_pointer(type);
    token = token->next;
  }

  *rest = token;
  return type;
}

Type *read_type_tokens_with_pars(Token **rest, Token *token) {
  int pars = 0;
  while (equal(token, "(")) {
    token = token->next;
    pars++;
  }

  Type *type = read_type_tokens(&token, token);
  if (!type) {
    return NULL;
  }

  for (int i=0; i<pars; i++) {
    if (!equal(token, ")")) {
      error_at(token, "expected )");
    }
    token = token->next;
  }

  *rest = token;
  return type;
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
  if (node->fncl) {
    add_type(node->fncl->args);
  }

  switch (node->kind) {
  // statement
  case ND_BLOCK:
  case ND_IF:
  case ND_WHILE:
  case ND_FOR:
  case ND_RETURN:
  case ND_EXPR_STMT:
  case ND_DECLARE_LVAR:
    return;

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
    node->type = new_type_pointer(node->left->type);
    return;
  case ND_DEREF:
    node->type = node->left->type->ptr_to;
    if (!node->type) {
      error("a expression refer to unexist entity");
    }
    return;
  default:
    error("faild to add type because of unknown Node.kind");
  }
}
