#include "parse.h"

Var *gvars;
Var *lvars;

Var *find_var(char *name, int length, Var *vars) {
  for (Var *var = vars; var; var = var->next) {
    if (length == var->length && !strncmp(name, var->name, length))
      return var;
  }
  return NULL;
}

void *new_var(Type *type, char *name, int length, Var **varsp) {
  Var *vars = *varsp;
  int already_reserved_offset = (vars ? (vars->offset ) : 0);

  Var *var = calloc(1, sizeof(Var));
  var->type = type;
  var->next = vars;
  var->name = name;
  var->length = length;
  var->offset = type_size(type) + already_reserved_offset;

  *varsp = var;
}
