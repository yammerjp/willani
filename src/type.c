#include "willani.h"

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

Type *new_type_array(Type *parent, size_t array_size) {
  Type *type = calloc(1, sizeof(Type));
  type->kind = TYPE_ARRAY;
  type->ptr_to = parent;
  type->array_size = array_size;
  return type;
}

const int type_size_pointer = 8;

int type_size(Type *type) {
  switch (type->kind) {
    case TYPE_INT:
      return 8;
    case TYPE_PTR:
      return type_size_pointer;
    case TYPE_ARRAY:
      return type->array_size;
    default:
      error("unknown type size");
  }
}

Type *read_type_tokens(Token **rest, Token *token) {
  Type *type;
  if(equal(token, "int")) {
    type = new_type_int();
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
static Type *type_conversion(Type *left, Type *right) {
  if (left->kind == TYPE_PTR || left->kind == TYPE_ARRAY) {
    return left;
  }
  if (right->kind == TYPE_PTR || right->kind == TYPE_ARRAY) {
    return right;
  }
  if (type_size(left) >= type_size(right)) {
    return left;
  }
  return right;
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
  case ND_GVAR:
  case ND_LVAR:
    node->type = node->var->type;
    return;
  case ND_ADD:
  case ND_SUB:
  case ND_MUL:
  case ND_DIV:
  case ND_EQ:
  case ND_NE:
  case ND_LT:
  case ND_LE:
    node->type = type_conversion(node->left->type, node->right->type);
    return;
  case ND_ASSIGN:
    node->type = node->left->type;
    return;
  case ND_NUM:
    node->type = new_type_int();
    return;
  case ND_FUNC_CALL:
    // TODO: recognize calling function's type
    node->type = new_type_int();
    return;
  case ND_ADDR:
    node->type = new_type_pointer(node->left->type);
    return;
  case ND_DEREF:
    node->type = node->left->type->ptr_to;
    return;
  default:
    error("faild to add type because of unknown Node.kind");
  }
}
