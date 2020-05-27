#include "willani.h"

Type *new_type(TypeKind kind, int pointer_depth) {
  if (kind != TP_INT) {
    error("unexpected type");
  }
  Type *type = calloc(1, sizeof(Type));
  type->kind = TP_INT;

  for(int i=0; i<pointer_depth; i++) {
    Type *new_type = calloc(1, sizeof(Type));
    new_type->kind = TP_PTR;
    new_type->ptr_to = type;
    type = new_type;
  }
  return type;
}

bool is_type_token(Token *token) {
  return equal(token, "int");
}

int type_size(Type *type) {
  if (type->kind == TP_INT) {
    return 8;
  }
  if (type->kind == TP_PTR) {
    return 8;
  }
  error("enexpected type");
}
