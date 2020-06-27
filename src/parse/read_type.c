#include "parse.h"

bool is_type_tokens(Token *token, AllowStaticOrNot ason) {
  return (bool) read_type(&token, token, ason);
}

Type *read_type(Token **rest, Token *token, AllowStaticOrNot allow_static_or_not) {
  Type *type;

  bool is_static = false;
  if (allow_static_or_not == ALLOW_STATIC && equal(token, "static")) {
    token = token->next;
    is_static = true;
  }
  
  if(equal(token, "long")) {
    type = new_type_long();
    token = token->next;
  } else if(equal(token, "int")) {
    type = new_type_int();
    token = token->next;
  } else if(equal(token, "char")) {
    type = new_type_char();
    token = token->next;
  } else if(equal(token, "_Bool")) {
    type = new_type_bool();
    token = token->next;
  } else if(equal(token, "struct")) {
    type = read_new_type_struct(&token, token->next);
  } else if (is_identifer_token(token)) {
    TypeDef *tdf = find_typedef(token->location, token->length);
    if (!tdf)
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

  type->is_static = is_static;
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
  Type *type = read_type(&token, token, DENY_STATIC);

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
