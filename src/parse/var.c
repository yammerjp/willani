#include "parse.h"

Var *gvars;
Var *lvars;
Var *outer_scope_lvars;

Var *find_typedef(char *name, int length, Var *head, Var *ignore) {
  for (Var *var = head; var && var != ignore; var = var->next) {
    if (!var->referable || !var->is_typedef)
      continue;
    if (length == var->length && !strncmp(name, var->name, length))
      return var;
  }
  return NULL;
}

Var *find_var(char *name, int length, Var *head, Var *ignore) {
  for (Var *var = head; var && var != ignore; var = var->next) {
    if (!var->referable || var->is_typedef)
      continue;
    if (length == var->length && !strncmp(name, var->name, length))
      return var;
  }
  return NULL;
}

void *new_typedef(Type *type, char *name, int namelen, Var **varsp) {
  Var *vars = *varsp;
  int already_reserved_offset = (vars ? (vars->offset ) : 0);

  Var *var = calloc(1, sizeof(Var));
  var->type = type;
  var->next = vars;
  var->name = name;
  var->length = namelen;
  var->offset = already_reserved_offset;
  var->is_typedef = true;
  var->referable = true;

  *varsp = var;
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
  var->is_typedef = false;
  var->referable = true;

  *varsp = var;
}
