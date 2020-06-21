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

Type *new_type_struct(int size, Member *members) {
  Type *type = calloc(1, sizeof(Type));
  type->kind = TYPE_STRUCT;
  type->size = size;
  type->members = members;
  return type;
}

Member *new_member(char *name, int namelen, Type *type, int offset) {
  Member *member = calloc(1, sizeof(Member));
  member->name = name;
  member->namelen = namelen;
  member->type = type;
  member->offset = offset;
  return member;
}

Member *find_member(Type *type, char *name, int namelen) {
  for (Member *cur = type->members; cur; cur = cur->next) {
    if (cur->namelen == namelen && strncmp(cur->name, name, namelen) == 0)
      return cur;
  }
  return NULL;
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
