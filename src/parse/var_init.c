#include "parse.h"

static ArrayIndexes *add_descendant(ArrayIndexes *now_descendant, int index) {
  ArrayIndexes *descendant = calloc(1, sizeof(ArrayIndexes));
  descendant->index = index;
  descendant->parent = now_descendant;
  return descendant;
}

static int var_array_length(ArrayIndexes *indexes) {
  Type *type = lvars->type;
  while(indexes) {
    indexes = indexes->parent;
    type = type->base;
  }
  return type->array_length;
}

static Node *new_node_array_cell(ArrayIndexes *indexes, Token *token) {
  if (!indexes)
    return new_node_var(lvars->name, lvars->length, token);

  return new_node_deref(new_node_add(
    new_node_array_cell(indexes->parent, token),
    new_node_num(indexes->index, token),
    token
  ),token);
}

static Node *new_node_zero_padding_array(ArrayIndexes *descendant, Token *token) {
  Node head = {};
  Node *tail = &head;

  ArrayIndexes *indexes = descendant;
  Type *type = lvars->type;
  while(indexes) {
    indexes = indexes->parent;
    type = type->base;
    if (!type)
      error("too deep initializer of array");
  }

  if (type->kind != TYPE_ARRAY)
    return new_node_expr_stmt(new_node_assign(new_node_array_cell(descendant, token), new_node_num(0, token), token), token);

  for (int i = 0; i < type->array_length; i++) {
    tail->next = new_node_zero_padding_array(add_descendant(descendant, i), token);
    while (tail->next)
      tail = tail->next;
  }
  return head.next;
}

Node *init_lvar_stmts(Token **rest, Token *token, ArrayIndexes *descendant) {
  Node *node;

  if (!equal(token, "{")) {
    Node *right = assign(&token, token);
    node = new_node_expr_stmt(new_node_assign(new_node_array_cell(descendant, token), right, token), token);
    *rest = token;
    return node;
  }
  token = token->next;

  Node head = {};
  Node *tail = &head;
  int array_length = var_array_length(descendant);
  int ct = 0;

  while (!equal(token, "}")) {
    tail->next = init_lvar_stmts(&token, token, add_descendant(descendant, ct++));
    while(tail->next)
      tail = tail->next;

    if (!equal(token,","))
      break;
    token = token->next;

    if (ct > array_length)
      error_at(token->location, "too many initializer of array");
  }

  if (!equal(token, "}"))
    error_at(token->location, "expected }");
  token = token->next;

  // Zero padding
  for (;ct< array_length; ct++) {
    tail->next = new_node_zero_padding_array(add_descendant(descendant, ct), token);
    while(tail->next)
      tail = tail->next;
  }

  node = head.next;
  *rest = token;
  return node;
}
