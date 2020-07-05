#include "parse.h"

static Node *read_array(Token **rest, Token *token, Type *type, long *values, int offset, Var *var);
static Node *read_cell(Token **rest, Token *token, long *values, int offset, Var *var);
static void *read_array_string(Token **rest, Token *token, Type *type, long *values, int offset, Var *var);

// declare and initialize variable statement = type declarator type_suffix "=" read_var_init ";"
// read_var_init                             = num | "{" read_var_init ("," read_var_init)* ","? "}"
Node *read_var_init(Token **rest, Token *token, Var *var) {
  // measure size
  int size = 1;
  for (Type *ty = var->type; ty->kind == TYPE_ARRAY; ty = ty->base)
    size *= ty->array_length;

  long *values = calloc(size, sizeof(long));

  Node *runtime_inits = read_array(rest, token, var->type, values, 0, var);
  if (var->is_global && runtime_inits)
    error_at(token, "failed that global variable is initilized by run-time-settled value");

  var->init_values = values;
  var->init_size = size;

  Node *node = new_node_var_init(token, var, values, size);
  node->next = runtime_inits;
  return node;
}

static Node *read_array(Token **rest, Token *token, Type *type, long *values, int offset, Var *var) {
  Node head = {};
  Node *tail = &head;

  // array cell init
  if (type->kind != TYPE_ARRAY)
    return read_cell(rest, token, values, offset, var);

  // array row init by string
  if (type->base->kind != TYPE_ARRAY && is_string_token(token)) {
    read_array_string(rest, token, type, values, offset, var);
    return NULL;
  }

  // array init recursive
  if (!equal(token, "{"))
    error_at(token, "expected { to initialize variable");
  token = token->next;

  int cell_size = 1;
  for (Type *ty = type->base; ty->kind == TYPE_ARRAY; ty = ty->base)
    cell_size *= ty->array_length;

  for (int i=0; i < type->array_length; i++) {
    if (equal(token, "}"))
      break;
    tail->next = read_array(&token, token, type->base, values, offset + (cell_size * i), var);
    while (tail->next)
      tail = tail->next;
    if (equal(token, "}"))
      break;
    if (!equal(token, ","))
      error_at(token, "expected , to initialize variable");
    token = token->next;
  }

  if (!equal(token, "}"))
    error_at(token, "expected } to initialize variable");
  token = token->next;

  *rest = token;
  return head.next;
}

static Node *read_cell(Token **rest, Token *token, long *values, int offset, Var *var) {
  if (is_number_token(token)) {
    // array cell init by number
    values[offset] = str_to_l(token->location, token->length);
    *rest = token->next;
    return NULL;
  }
  if (var->is_global)
    error_at(token, "expected number token to initialize variable");

  // array cell init by assign
  Node *left = new_node_var_specified(var, token);
  while (left->type->kind == TYPE_ARRAY)
    left = new_node_deref(left, token);

  Node *right = assign(&token, token);
  *rest = token;
  return new_node_expr_stmt(new_node_assign(left, right, token), token);
}

static void *read_array_string(Token **rest, Token *token, Type *type, long *values, int offset, Var *var) {
  String *string = new_string(token->location+1, token->length-2);
  if (type->array_length < string->length)
    error_at(token, "string token is too longer than array length");

  for (int i=0; i < string->length; i++)
    values[offset+i] = (string->p)[i];
  *rest = token->next;
}
