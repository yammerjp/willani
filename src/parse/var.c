#include "parse.h"

Var *gvars;

int lvar_byte;

Var *find_var(char *name, int namelen, Var *vars, Var *outer_scope_vars, IncludeTypedef include_typedef) {
  for (Var *var = vars; var && var != outer_scope_vars; var = var->next) {
    if (include_typedef ==  EXCLUDE_TYPEDEF && var->is_typedef)
      continue;
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
