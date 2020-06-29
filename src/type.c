#include "willani.h"

const int type_size_pointer = 8;

static Type *new_type(TypeKind kind, int size) {
  Type *type = calloc(1, sizeof(Type));
  type->kind = kind;
  type->size = size;
  return type;
}

Type *new_type_long() {
  return new_type(TYPE_LONG, 8);
}

Type *new_type_int() {
  return new_type(TYPE_INT, 4);
}

Type *new_type_char() {
  return new_type(TYPE_CHAR, 1);
}

Type *new_type_bool() {
  return new_type(TYPE_BOOL, 1);
}

Type *new_type_pointer(Type *parent) {
  Type *type = new_type(TYPE_PTR, type_size_pointer);
  type->base = parent;
  return type;
}

Type *new_type_array(Type *parent, int length) {
  Type *type = new_type(TYPE_ARRAY, length * parent->size);
  type->base = parent;
  type->array_length  = length;
  return type;
}
Type *new_type_enum() {
  return new_type(TYPE_ENUM, 4);
}

Type *new_type_struct(int size, Member *members) {
  Type *type = new_type(TYPE_STRUCT, size);
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
  for (Member *mem = type->members; mem; mem = mem->next) {
    if (mem->namelen == namelen && strncmp(mem->name, name, namelen) == 0)
      return mem;
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

bool same_type(Type *t1, Type *t2) {
  if (!t1 || !t2)
    error("comparing type is NULL");

  if ( t1->kind != t2->kind || t1->array_length != t2->array_length || t1->is_static != t2->is_static)
    return false;

  if (t1->kind == TYPE_ARRAY || t1->kind == TYPE_PTR)
    return same_type(t1->base, t2->base);

  return true;
}

bool is_ptr_or_arr(Type *type) {
  return type->kind == TYPE_PTR || type->kind == TYPE_ARRAY;
}
