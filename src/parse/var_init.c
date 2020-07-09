#include "parse.h"

static Node *read_cells(Token **rest, Token *token, Type *type, char *values, int offset, Var *var);
static Node *read_array(Token **rest, Token *token, Type *type, char *values, int offset, Var *var);
static Node *read_struct(Token **rest, Token *token, Type *type, char *values, int offset, Var *var);
static Node *read_cell(Token **rest, Token *token, Type *type, char *values, int offset, Var *var);
static void *read_array_string(Token **rest, Token *token, Type *type, char *values, int offset, Var *var);

// declare and initialize variable statement = type declarator type_suffix "=" read_var_init ";"
// read_var_init                             = num | "{" read_var_init ("," read_var_init)* ","? "}"
Node *read_var_init(Token **rest, Token *token, Var *var) {
  // measure size
  int size = var->type->size;

  char *values = calloc(size, sizeof(char));

  Node *runtime_inits = read_cells(rest, token, var->type, values, 0, var);
  if (var->is_global && runtime_inits)
    error_at(token, "failed that global variable is initilized by run-time-settled value");

  var->init_values = values;
  var->init_size = size;

  Node *node = new_node_var_init(token, var, values, size);
  node->next = runtime_inits;
  return node;
}

static Node *read_cells(Token **rest, Token *token, Type *type, char *values, int offset, Var *var) {
  if (type->kind == TYPE_ARRAY)
    return read_array(rest, token, type, values, offset, var);
  if (type->kind == TYPE_STRUCT)
    return read_struct(rest, token, type, values, offset, var);

  return read_cell(rest, token, type, values, offset, var);
}

static Node *read_array(Token **rest, Token *token, Type *type, char *values, int offset, Var *var) {
  Node head = {};
  Node *tail = &head;

  // array row init by string
  if (is_string_token(token)) {
    if (type->base->kind == TYPE_ARRAY || type->base->kind == TYPE_STRUCT)
      error_at(token, "expected { of array or struct initialization");
    read_array_string(rest, token, type, values, offset, var);
    return NULL;
  }

  // array init recursive
  if (!equal(token, "{"))
    error_at(token, "expected { to initialize variable");
  token = token->next;

  int cell_size = type->base->size;

  for (int i=0; i < type->array_length; i++) {
    if (equal(token, "}"))
      break;
    tail->next = read_cells(&token, token, type->base, values, offset + (cell_size * i), var);
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

static Node *read_struct(Token **rest, Token *token, Type *type, char *values, int offset, Var *var) {
  Node head = {};
  Node *tail = &head;

  // array init recursive
  if (!equal(token, "{"))
    error_at(token, "expected { to initialize variable");
  token = token->next;

  for (Member *m = type->members; m; m=m->next) {
    if (equal(token, "}"))
      break;
    tail->next = read_cells(&token, token, m->type, values, offset + m->offset, var);
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

static Node *read_cell(Token **rest, Token *token, Type *type, char *values, int offset, Var *var) {
  if ( is_number_token(token)
    && (equal(token->next, "}") || equal(token->next, ";") || equal(token->next, ",")) ) {

    int size = type->size;
    long val = str_to_l(token->location, token->length);
    // array cell init by number
    for (int i=0; i < size; i++) {
      values[offset+i] = val;
      val >>= 8; // x86_64 is little endian
    }
    *rest = token->next;
    return NULL;
  }
  if (var->is_global)
    error_at(token, "expected number token to initialize variable");

  // array cell init by assign that settled runtime
  Var *var_cell = calloc(1, sizeof(Var));
  var_cell->type = type;
  var_cell->offset = var->offset - offset;

  Node *left = new_node_var_specified(var_cell, token);
  while (left->type->kind == TYPE_ARRAY)
    left = new_node_deref(left, token);

  Node *right = assign(&token, token);
  *rest = token;
  return new_node_expr_stmt(new_node_assign(left, right, token), token);
}

static void *read_array_string(Token **rest, Token *token, Type *type, char *values, int offset, Var *var) {
  String *string = new_string(token->location+1, token->length-2);
  if (type->array_length < string->length)
    error_at(token, "string token is too longer than array length");
  int charactor_size = type->base->size;

  for (int i=0; i < string->length; i++)
    values[offset + (i * charactor_size)] = (string->p)[i];
  *rest = token->next;
}
