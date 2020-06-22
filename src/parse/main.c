#include "parse.h"

static void read_new_gvar(Token **rest, Token *token, Type *type_without_suffix, char *name, int namelen);

// program = (function | declare_gvar | typedef_stmt)*
// declare_gvar = type ident type_suffix ";"

void *program(Token *token) {
  while (!is_eof_token(token)) {
    // "typedef" type identifer ";"
    if (equal(token, "typedef")) {
      typedef_stmt(&token, token, &gvars);
      continue;
    }

    // type
    Type *type = read_type(&token, token);
    if (!type)
      error_at(token->location, "unexpected type");

    // function name
    if (!is_identifer_token(token))
      error_at(token->location, "expected identifer of function name or global variable");

    char *name = token->location;
    int namelen = token->length;
    token = token->next;

    if (!equal(token, "(")) {
      // global variable
      read_new_gvar(&token, token, type, name, namelen);
      continue;
    }

    // function
    Function *func_samename = find_function(name, namelen);
    Function *func = function_definition(&token, token, type, name, namelen);
    if (func_samename && !cmp_function(func, func_samename))
      error_at(token->location, "type is conflict with the same name function definition");

    add_function(func);
    if (equal(token, ";")) {
      token = token->next;
      func->definition = true;
      continue;
    }
    if (func->definition)
      error_at(token->location, "need arguments' identifer");

    if (func_samename && !func_samename->definition)
      error_at(token->location, "a entitiy of the same name function is exist");

    lvars = func->args;
    func->node = block_stmt(&token, token);
    func->var = lvars;
  }
  parse_log();
}

static void read_new_gvar(Token **rest, Token *token, Type *type_without_suffix, char *name, int namelen) {
  Type *type = type_suffix(&token, token, type_without_suffix);
  if (find_var(name, namelen, gvars, NULL))
    error_at(token->location, "duplicate declarations");
  new_var(type, name, namelen, &gvars);

  if (!equal(token, ";"))
    error_at(token->location, "expected ;");
  token = token->next;

  *rest = token;
}
