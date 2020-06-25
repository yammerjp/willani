#include "parse.h"

Var *gvars;

int lvar_byte;

Var *find_var_in_vars(char *name, int namelen, Var *vars) {
  for (Var *var = vars; var; var = var->next) {
    if (namelen == var->namelen && !strncmp(name, var->name, namelen))
      return var;
  }
  return NULL;
}

static void *new_struct_var(Type *type, char *name, int namelen, bool is_typedef) {
  if (type->is_static)
    error_at(name, "not support static variables");
  int size = is_typedef ? 0 : type->size;
  lvar_byte += size;

  Var *var = calloc(1, sizeof(Var));
  var->type = type;
  var->next = now_scope->vars;
  var->name = name;
  var->namelen = namelen;
  var->offset = lvar_byte;
  var->is_typedef = is_typedef;
  var->is_global = !(now_scope->parent);

  now_scope->vars = var;
}

void *new_typedef(Type *type, char *name, int namelen) {
  return new_struct_var(type, name, namelen, true);
}

void *new_var(Type *type, char *name, int namelen) {
  return new_struct_var(type, name, namelen, false);
}
