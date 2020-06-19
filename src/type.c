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

Type *new_type_bool() {
  Type *type = calloc(1, sizeof(Type));
  type->kind = TYPE_BOOL;
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

Type *new_type_struct(Token **rest, Token *token) {
  if (!equal(token, "{"))
    error_at(token, "expected {");
  token = token->next;

  Member head;
  Member *tail = &head;
  int offset = 0;
  while (!equal(token, "}")) {
    tail->next = read_member(&token, token, offset);
    tail = tail->next;
    offset += tail->type->size;
  }
  token = token->next;

  Type *type = calloc(1, sizeof(Type));
  type->kind = TYPE_STRUCT;
  type->size = offset;
  type->members = head.next;

  *rest = token;
  return type;
}

Member *read_member(Token **rest, Token *token, int offset) {
  Type *type = read_type(&token, token);

  if (!is_identifer_token(token))
    error_at(token, "expected identifer");
  char *name = token->location;
  int namelen = token->length;
  token = token->next;

  type = type_suffix(&token, token, type);

  if (!equal(token, ";"))
    error_at(token, "expected ;");
  token = token->next;

  Member *member = calloc(1, sizeof(Member));
  member->name = name;
  member->namelen = namelen;
  member->type = type;
  member->offset = offset;

  *rest = token;
  return member;
}

Member *find_member(Type *type, char *name, int namelen) {
  for (Member *cur = type->members; cur; cur = cur->next) {
    if (cur->namelen == namelen && strncmp(cur->name, name, namelen) == 0)
      return cur;
  }
  return NULL;
}

Type *read_type(Token **rest, Token *token) {
  Type *type;
  if(equal(token, "long")) {
    type = new_type_long();
    token = token->next;
  } else if(equal(token, "int")) {
    type = new_type_int();
    token = token->next;
  } else if(equal(token, "char")) {
    type = new_type_char();
    token = token->next;
  } else if(equal(token, "bool")) {
    type = new_type_bool();
    token = token->next;
  } else if(equal(token, "struct")) {
    type = new_type_struct(&token, token->next);
  } else {
    return NULL;
  }

  while (equal(token, "*")) {
    type = new_type_pointer(type);
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
