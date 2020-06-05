#include "parse.h"

// program = (function | declare_gvar)*
// declare_gvar = type ident type_suffix ";"
void *program(Token *token) {
  Function head = {};
  Function *tail = &head;

  while (!is_eof_token(token)) {

    // type
    Type *type = read_type_tokens(&token, token);

    // function name
    if (!is_identifer_token(token)) {
      error_at(token, "expected identifer");
    }

    char *name = token->location;
    int namelen = token->length;
    token = token->next;

    if (equal(token, "(")) {
      // function
      tail->next = function(&token, token, type, name, namelen);
      tail = tail->next;
    } else {
      // global variable
      type = type_suffix(&token, token, type);
      if (find_var(name, namelen, gvars)) {
        error_at(token, "duplicate declarations '%.*s'", namelen, name);
      }
      new_var(type, name, namelen, &gvars);

      if (!equal(token, ";")) {
        error_at(token, "expected ;");
      }
      token = token->next;
    }
  }

  functions = head.next;
  parse_log();
}

// function = type ident "(" ( ( type ident ( "," type ident ) * ) ?  ")" block_stmt
Function *function(Token **rest, Token *token, Type *return_type, char *name, int namelen) {
  Var *lvars = NULL;

  // arguments
  if (!equal(token, "(")) {
    error_at(token, "expected (");
  }
  token = token->next;

  int argc = 0;

  while (!equal(token, ")")) {
    Type *arg_type = read_type_tokens(&token, token);

    argc++;

    if(!is_identifer_token(token)) {
      error_at(token, "expected identifer");
    }
    new_var(arg_type, token->location, token->length, &lvars);
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

  Var *args = lvars;

  // block statement
  Node *node = block_stmt(&token, token, &lvars);

  // create Function struct
  Function *func = calloc(1, sizeof(Function));
  func->node = node;
  func->var = lvars;
  func->name = name;
  func->args = args;
  func->argc = argc;
  func->namelen = namelen;
  func->type = return_type;

  *rest = token;
  return func;
}
