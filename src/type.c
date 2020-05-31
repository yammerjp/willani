#include "willani.h"

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

  return new_type_int();
}
