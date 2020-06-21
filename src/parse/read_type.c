#include "parse.h"

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
    type = read_new_type_struct(&token, token->next);
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

Type *read_new_type_struct(Token **rest, Token *token) {
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
  *rest = token->next;
  return new_type_struct(offset, head.next);
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

  *rest = token;
  return new_member(name, namelen, type, offset);
}
