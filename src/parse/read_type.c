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
  } else if (is_identifer_token(token)) {
    Var *tdf = find_var(token->location, token->length, lvars, NULL);
    if (!tdf)
      tdf = find_var(token->location, token->length, gvars, NULL);
    if (!tdf || !tdf->is_typedef)
      return NULL;
    token = token->next;
    type = tdf->type;
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

// ident? ( "{" member "}" )? ;
Type *read_new_type_struct(Token **rest, Token *token) {
  char *name = NULL;
  int namelen;
  if (is_identifer_token(token)) {
    name = token->location;
    namelen = token->length;
    token = token->next;
  }

  if (!equal(token, "{")) {
    Tag *tag = find_tag(name, namelen);
    if (!tag)
      error_at(token->location, "called a undefined tag of struct");
    *rest = token;
    return tag->type;
  }
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


  Type *type = new_type_struct(offset, head.next);
  if (name) {
    new_tag(name, namelen, type);
  }

  *rest = token;
  return type;
}

Member *read_member(Token **rest, Token *token, int offset) {
  Type *type = read_type(&token, token);

  if (!is_identifer_token(token))
    error_at(token->location, "expected member identifer");
  char *name = token->location;
  int namelen = token->length;
  token = token->next;

  type = type_suffix(&token, token, type);

  if (!equal(token, ";"))
    error_at(token->location, "expected ;");
  token = token->next;

  *rest = token;
  return new_member(name, namelen, type, offset);
}
