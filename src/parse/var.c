#include "parse.h"

Var *gvars;

int lvar_byte;

typedef enum {
  ALLOW_ALL,
  ONLY_EXTERN,
  ONLY_ENTITY,
} Options;

static Var *find_in_vars_with_option(char *name, int namelen, Var *vars, Options opt) {
  for (Var *var = vars; var; var = var->next) {
    if ( namelen == var->namelen && !strncmp(name, var->name, namelen)
      && ( opt == ALLOW_ALL
        || opt == ONLY_EXTERN && var->type->is_extern
        || opt == ONLY_ENTITY && !var->type->is_extern
    ))
      return var;
  }
  return NULL;
}

Var *find_in_vars(char *name, int namelen, Var *vars) {
  return find_in_vars_with_option(name, namelen, vars, ALLOW_ALL);
}

Var *find_in_vars_of_extern(char *name, int namelen, Var *vars) {
  return find_in_vars_with_option(name, namelen, vars, ONLY_EXTERN);
}

Var *find_in_vars_without_extern(char *name, int namelen, Var *vars) {
  return find_in_vars_with_option(name, namelen, vars, ONLY_ENTITY);
}

void *new_var(Type *type, char *name, int namelen) {
  bool is_global =  !(now_scope->parent);
  // TODO support local static variables
  if (!is_global && type->is_static)
    error_at(name, "not support local static variables");
  lvar_byte += type->size;

  Var *var = calloc(1, sizeof(Var));
  var->type = type;
  var->next = now_scope->vars;
  var->name = name;
  var->namelen = namelen;
  var->offset = lvar_byte;
  var->is_global = is_global;

  now_scope->vars = var;
}
