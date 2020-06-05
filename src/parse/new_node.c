#include "parse.h"

static Node *new_node_op2(NodeKind kind, Node *left, Node *right) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = kind;
  node->left = left;
  node->right = right;
  node->type = type_conversion(node->left->type, node->right->type);
  return node;
}

Node *new_node_add(Node *left, Node *right) {
  if (
    left->type->kind == TYPE_PTR
    || left->type->kind == TYPE_ARRAY
  ) {
    right = new_node_mul(right, new_node_num( type_size(left->type->ptr_to) ));
  } else if (
    right->type->kind == TYPE_PTR
    || right->type->kind == TYPE_ARRAY
  ) {
    left = new_node_mul(left, new_node_num( type_size(right->type->ptr_to) ));
  }
  return new_node_op2(ND_ADD, left, right);
}

Node *new_node_sub(Node *left, Node *right) {
  if (
    left->type->kind == TYPE_PTR
    || left->type->kind == TYPE_ARRAY
  ) {
    right = new_node_mul(right, new_node_num( type_size(left->type->ptr_to) ));
  } else if (
    right->type->kind == TYPE_PTR
    || right->type->kind == TYPE_ARRAY
  ) {
    left = new_node_mul(left, new_node_num( type_size(right->type->ptr_to) ));
  }
  return new_node_op2(ND_SUB, left, right);
}

Node *new_node_mul(Node *left, Node *right) {
  return new_node_op2(ND_MUL, left, right);
}

Node *new_node_div(Node *left, Node *right) {
  return new_node_op2(ND_DIV, left, right);
}

Node *new_node_equal(Node *left, Node *right) {
  Node *node = new_node_op2(ND_EQ, left, right);
  node->type = new_type_int();
  return node;
}

Node *new_node_not_equal(Node *left, Node *right) {
  Node *node = new_node_op2(ND_NE, left, right);
  node->type = new_type_int();
  return node;
}

Node *new_node_less_than(Node *left, Node *right) {
  Node *node = new_node_op2(ND_LT, left, right);
  node->type = new_type_int();
  return node;
}

Node *new_node_less_equal(Node *left, Node *right) {
  Node *node = new_node_op2(ND_LE, left, right);
  node->type = new_type_int();
  return node;
}

Node *new_node_num(long value) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_NUM;
  node->value = value;
  node->type = new_type_int();
  return node;
}

Node *new_node_string(char *p, int length) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_STRING;
  node->string = new_string(p, length);
  node->type = new_type_pointer(new_type_char());
  return node;
}

Node *new_node_var(char *name, int length, Var *lvars) {
  Node *node = calloc(1, sizeof(Node));

  Var *gvar = find_var(name, length, gvars);
  if (gvar) {
    node->kind = ND_GVAR;
    node->var = gvar;
    node->type = gvar->type;
    return node;
  }

  Var *lvar = find_var(name, length, lvars);
  if (lvar) {
    node->kind = ND_LVAR;
    node->var = lvar;
    node->type = lvar->type;
    return node;
  }

  error("use undeclared identifer '%.*s'", length, name);
}

Node *new_node_return(Node *left) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_RETURN;
  node->left = left;
  return node;
}

Node *new_node_if(Node *cond, Node *then, Node *els) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_IF;
  node->cond = cond;
  node->then = then;
  node->els = els;
  return node;
}

Node *new_node_while(Node *cond, Node *then) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_WHILE;
  node->cond = cond;
  node->then = then;
  return node;
}

Node *new_node_for(Node *init, Node *cond, Node* increment, Node *then) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_FOR;
  node->init = init;
  node->increment = increment;
  node->cond = cond;
  node->then = then;
  return node;
}

Node *new_node_block(Node *body) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_BLOCK;
  node->body = body;
  return node;
}

Node *new_node_func_call(char *name, int len, Node *args) {
  Function *func = find_function(name, len);
  if (!func) {
    error("called undefined function '%.*s'", len, name);
  }

  FuncCall *fncl = calloc(1, sizeof(FuncCall));
  fncl->name = name;
  fncl->args = args;
  fncl->length = len;
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_FUNC_CALL;
  node->fncl = fncl;
  node->type = func->type;

  return node;
}

Node *new_node_expr_stmt(Node *stmt_node) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_EXPR_STMT;
  node->left = stmt_node;
  return node;
}

Node *new_node_addr(Node *unary_node) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_ADDR;
  node->left = unary_node;
  node->type = new_type_pointer(unary_node->type);
  return node;
}

Node *new_node_deref(Node *unary_node) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_DEREF;
  node->left = unary_node;
  node->type = unary_node->type->ptr_to;
  return node;
}

Node *new_node_assign(Node *left, Node *right) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_ASSIGN;
  node->left = left;
  node->right = right;
  node->type = left->type;
  return node;
}
