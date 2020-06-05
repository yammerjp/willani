#include "parse.h"

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
