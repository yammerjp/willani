#include "parse.h"

Var *gvars;
Var *lvars;
Var *outer_scope_lvars;

Var *find_var(char *name, int namelen, Var *vars, Var *outer_scope_vars, IncludeTypedef include_typedef) {
  for (Var *var = vars; var && var != outer_scope_vars; var = var->next) {
    if (!var->referable || include_typedef ==  EXCLUDE_TYPEDEF && var->is_typedef)
      continue;
    if (namelen == var->namelen && !strncmp(name, var->name, namelen))
      return var;
  }
  return NULL;
}

static void *new_struct_var(Type *type, char *name, int namelen, Var **varsp, bool is_typedef) {
  Var *vars = *varsp;
  int already_reserved_offset = (vars ? (vars->offset ) : 0);
  int size = is_typedef ? 0 : type->size;

  Var *var = calloc(1, sizeof(Var));
  var->type = type;
  var->next = vars;
  var->name = name;
  var->namelen = namelen;
  var->offset = size + already_reserved_offset;
  var->is_typedef = is_typedef;
  var->referable = true;

  *varsp = var;
}

void *new_typedef(Type *type, char *name, int namelen, Var **varsp) {
  return new_struct_var(type, name, namelen, varsp, true);
}

void *new_var(Type *type, char *name, int namelen, Var **varsp) {
  return new_struct_var(type, name, namelen, varsp, false);
}
