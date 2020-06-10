#include "willani.h"

const int type_size_pointer = 8;

Type *new_type_long() {
  Type *type = calloc(1, sizeof(Type));
  type->kind = TYPE_LONG;
  type->size = 8;
  return type;
}

Type *new_type_int() {
  Type *type = calloc(1, sizeof(Type));
  type->kind = TYPE_INT;
  type->size = 4;
  return type;
}

Type *new_type_char() {
  Type *type = calloc(1, sizeof(Type));
  type->kind = TYPE_CHAR;
  type->size = 1;
  return type;
}

Type *new_type_pointer(Type *parent) {
  Type *type = calloc(1, sizeof(Type));
  type->kind = TYPE_PTR;
  type->size = type_size_pointer;
  type->base = parent;
  return type;
}

Type *new_type_array(Type *parent, int length) {
  Type *type = calloc(1, sizeof(Type));
  type->kind = TYPE_ARRAY;
  type->size = length * parent->size;
  type->base = parent;
  type->array_length  = length;
  return type;
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

  if (left->size >= right->size)
    return left;

  return right;
}

bool cmp_type(Type *t1, Type *t2) {
  if (!t1 || !t2)
    error("comparing type is NULL");

  if ( t1->kind != t2->kind || t1->array_length != t2->array_length)
    return false;

  if (t1->kind == TYPE_ARRAY || t1->kind == TYPE_PTR)
    return cmp_type(t1->base, t2->base);

  return true;
}
