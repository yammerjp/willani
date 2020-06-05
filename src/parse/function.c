#include "parse.h"

Function *find_function(char *name, int namelen) {
  for (Function *cur = functions; cur; cur = cur->next) {
    if (strncmp(name, cur->name, namelen)==0 && cur->namelen == namelen) {
      return cur;
    }
  }
  return NULL;
}

void add_function(Function *func) {
  if (!functions) {
    functions = func;
    return;
  }

  Function *tail  = functions;
  while (tail->next) {
    tail = tail->next;
  }
  tail->next = func;
}

// function = type ident "(" ( ( type ident ( "," type ident ) * ) ?  ")" block_stmt
Function *function_definition(Token **rest, Token *token, Type *return_type, char *name, int namelen) {
  Var *args = NULL;
  bool definition = false;

  // arguments
  if (!equal(token, "(")) {
    error_at(token, "expected (");
  }
  token = token->next;

  int argc = 0;

  while (!equal(token, ")")) {
    Type *arg_type = read_type_tokens(&token, token);

    argc++;

    if (equal(token, ",")) {
      token = token->next;
      definition = true;
      new_var(arg_type, 0, 0, &args);
      continue;
    }
    if (equal(token, ")")) {
      definition = true;
      new_var(arg_type, 0, 0, &args);
      break;
    }

    if(!is_identifer_token(token)) {
      error_at(token, "expected identifer");
    }
    new_var(arg_type, token->location, token->length, &args);
    token = token->next;

    if (!equal(token, ",")) {
      break;
    }
    token = token->next;
  }

  if (!equal(token, ")")) {
    error_at(token, "expected )");
  }
  token = token->next;


  // create Function struct
  Function *func = calloc(1, sizeof(Function));
  func->name = name;
  func->args = args;
  func->argc = argc;
  func->namelen = namelen;
  func->type = return_type;
  func->definition = definition;

  *rest = token;
  return func;
}
