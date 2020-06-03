#include "willani.h"

Type *new_type_int() {
  Type *type = calloc(1, sizeof(Type));
  type->kind = TYPE_INT;
  return type;
}

Type *new_type_char() {
  Type *type = calloc(1, sizeof(Type));
  type->kind = TYPE_CHAR;
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
      return 4;
    case TYPE_CHAR:
      return 1;
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
  } else if(equal(token, "char")) {
    type = new_type_char();
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
Type *type_conversion(Type *left, Type *right) {
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
