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
