#include "willani.h"

// ========== lvar ==========
LVar *find_lvar(char *name, int length, LVar *lvars) {
  for (LVar *lvar = lvars; lvar; lvar = lvar->next) {
    if (length == lvar->length && !strncmp(name, lvar->name, length)) {
      return lvar;
    }
  }
  return NULL;
}

void *new_lvar(char *name, int length, LVar **lvarsp) {
  LVar *lvar = calloc(1, sizeof(LVar));
  lvar->next = *lvarsp;
  lvar->name = name;
  lvar->length = length;
  lvar->offset = *lvarsp ? ((*lvarsp)->offset + 8) : 8;

  *lvarsp = lvar;
}

// ========== new node ==========
Node *new_node_op2(NodeKind kind, Node *left, Node *right) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = kind;
  node->left = left;
  node->right = right;
  return node;
}

Node *new_node_num(long value) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_NUM;
  node->value = value;
  return node;
}

Node *new_node_var(char *name, int length, LVar *lvars) {
  LVar *lvar = find_lvar(name, length, lvars);
  if (!lvar) {
    error("use undeclared identifer '%.*s'", length, name);
  }
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_VAR;
  node->lvar = lvar;
  return node;
}

Node *new_node_declare_var(char *name, int length, LVar **lvarsp) {
  if (find_lvar(name, length, *lvarsp)!= NULL) {
    error("duplicate declarations '%.*s'", length, name);
  }

  new_lvar(name, length, lvarsp);

  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_DECLARE_VAR;
  node->lvar = *lvarsp;
  return node;
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
  FuncCall *fncl = calloc(1, sizeof(FuncCall));
  fncl->name = name;
  fncl->args = args;
  fncl->length = len;
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_FUNC_CALL;
  node->fncl = fncl;
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
  return node;
}

Node *new_node_deref(Node *unary_node) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_DEREF;
  node->left = unary_node;
  return node;
}
