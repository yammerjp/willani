#include "parse.h"

static void read_new_gvar(Token **rest, Token *token, Type *type_without_suffix, char *name, int namelen);

// program = (function | declare_gvar | typedef_stmt)*
// declare_gvar = type ident type_suffix ";"

void *program(Token *token) {
  scope_in();
  while (!is_eof_token(token)) {
    lvar_byte = 0;
    // "typedef" type identifer ";"
    if (equal(token, "typedef")) {
      typedef_stmt(&token, token);
      continue;
    }

    // type
    Type *type = read_type(&token, token, ALLOW_STATIC, ALLOW_EXTERN);
    if (!type)
      error_at(token, "unexpected type");
    if (equal(token, ";") && (type->kind == TYPE_ENUM || type->kind == TYPE_STRUCT)) {
      token = token->next;
      continue;
    }

    // function name
    if (!is_identifer_token(token))
      error_at(token, "expected identifer of function name or global variable");

    char *name = token->location;
    int namelen = token->length;
    token = token->next;

    if (!equal(token, "(")) {
      // global variable
      read_new_gvar(&token, token, type, name, namelen);
      continue;
    }
    if (type->is_extern)
      error_at(token, "the word 'extern' is allow only global variables declaration");

    // function
    if (find_in_vars(name, namelen, now_scope->vars) || find_in_typedefs(name, namelen, now_scope->tdfs)
      || find_in_enum_tags(name, namelen, now_scope->etags))
      error_at(token, "duplicate global declarations of variable/typedef/function/enum");
    scope_in();
    Function *func_samename = find_function(name, namelen);
    Function *func = function_definition(&token, token, type, name, namelen);
    if (func_samename && !same_function(func, func_samename))
      error_at(token, "type is conflict with the same name function definition");

    add_function(func);
    if (equal(token, ";")) {
      token = token->next;
      func->definition = true;
      scope_out();
      continue;
    }
    if (func->definition)
      error_at(token, "need arguments' identifer");

    if (func_samename && !func_samename->definition)
      error_at(token, "a entitiy of the same name function is exist");
    now_scope->vars = func->args;
    func->node = block_stmt(&token, token);
    func->var_byte = lvar_byte;

    scope_out();
  }
  // Todo memolize gvars (now_scope->vars)
  gvars = now_scope->vars;
  scope_out();
  parse_log();
}

static void read_new_gvar(Token **rest, Token *token, Type *type_without_suffix, char *name, int namelen) {
  Type *type = type_suffix(&token, token, type_without_suffix);

  if (find_in_typedefs(name, namelen, now_scope->tdfs) || find_function(name, namelen) || find_in_enum_tags(name, namelen, now_scope->etags)
  )
    error_at(token, "duplicate global declarations of typedef/function/enum");
  Var *varex = find_in_vars_of_extern(name, namelen, now_scope->vars);
  if (varex && !same_type(type, varex->type))
    error_at(token, "conflict the type of extern global variables declaration");
  Var *varen = find_in_vars_without_extern(name, namelen, now_scope->vars);
  if (varen && !type->is_extern)
    error_at(token, "duplicated global variables declaration");
  if (varen && !same_type(type, varen->type))
    error_at(token, "conflict the type of extern and entity global variables declaration");

  new_var(type, name, namelen);

  if (!equal(token, ";"))
    error_at(token, "expected ; to declare global variable statement");
  token = token->next;

  *rest = token;
}
