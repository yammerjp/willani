#include "parse.h"

static void add_function(Function *func);
static Function *function_definition(Token **rest, Token *token, Type *return_type, char *name, int namelen);

// program = (function | declare_gvar)*
// declare_gvar = type ident type_suffix ";"
void *program(Token *token) {

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
      Function *func = function_definition(&token, token, type, name, namelen);
      add_function(func);
      if (equal(token, ";")) {
        token = token->next;
        func->definition = true;
        continue;
      }
      if (func->definition) {
        error_at(token, "need arguments' identifer");
      }
      Var *lvars = func->args;
      func->node = block_stmt(&token, token, &lvars);
      func->var = lvars;
      continue;
    }

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
  parse_log();
}

static void add_function(Function *func) {
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
static Function *function_definition(Token **rest, Token *token, Type *return_type, char *name, int namelen) {
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
