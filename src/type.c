#include "willani.h"

Type *new_type_long() {
  Type *type = calloc(1, sizeof(Type));
  type->kind = TYPE_LONG;
  return type;
}

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

Type *new_type_array(Type *parent, int length) {
  Type *type = calloc(1, sizeof(Type));
  type->kind = TYPE_ARRAY;
  type->ptr_to = parent;
  type->array_length  = length;
  return type;
}

const int type_size_pointer = 8;

int type_size(Type *type) {
  switch (type->kind) {
    case TYPE_LONG:
      return 8;
    case TYPE_INT:
      return 4;
    case TYPE_CHAR:
      return 1;
    case TYPE_PTR:
      return type_size_pointer;
    case TYPE_ARRAY:
      return type->array_length * type_size(type->ptr_to);
    default:
      error("unknown type size");
  }
}

Type *read_type_tokens(Token **rest, Token *token) {
  Type *type;
  if(equal(token, "long"))
    type = new_type_long();
  else if(equal(token, "int"))
    type = new_type_int();
  else if(equal(token, "char"))
    type = new_type_char();
  else
    return NULL;

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
  if (!type)
    return NULL;

  for (int i=0; i<pars; i++) {
    if (!equal(token, ")"))
      error_at(token, "expected )");
    token = token->next;
  }

  *rest = token;
  return type;
}

// Add Type to all expression node after parsing
Type *type_conversion(Type *left, Type *right) {
  if (left->kind == TYPE_PTR || left->kind == TYPE_ARRAY)
    return left;

  if (right->kind == TYPE_PTR || right->kind == TYPE_ARRAY)
    return right;

  if (type_size(left) >= type_size(right))
    return left;

  return right;
}

bool cmp_type(Type *t1, Type *t2) {
  if (!t1 || !t2)
    error("comparing type is NULL");

  if ( t1->kind != t2->kind || t1->array_length != t2->array_length)
    return false;

  if (t1->kind == TYPE_ARRAY || t1->kind == TYPE_PTR)
    return cmp_type(t1->ptr_to, t2->ptr_to);

  return true;
}
