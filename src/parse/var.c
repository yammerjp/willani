#include "parse.h"

Var *gvars;
Var *lvars;

Var *find_var(char *name, int length, Var *head, Var *ignore) {
  for (Var *var = head; var && var != ignore; var = var->next) {
    if (!var->referable)
      continue;
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
  var->offset = type->size + already_reserved_offset;
  var->referable = true;

  *varsp = var;
}
