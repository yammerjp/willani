#include "willani.h"

// ========== lvar ==========
Var *find_lvar(char *name, int length, Var *lvars) {
  for (Var *lvar = lvars; lvar; lvar = lvar->next) {
    if (length == lvar->length && !strncmp(name, lvar->name, length)) {
      return lvar;
    }
  }
  return NULL;
}

void *new_lvar(Type *type, char *name, int length, Var **lvarsp) {
  int already_reserved_offset = (*lvarsp ? ((*lvarsp)->offset ) : 0);

  Var *lvar = calloc(1, sizeof(Var));
  lvar->type = type;
  lvar->next = *lvarsp;
  lvar->name = name;
  lvar->length = length;
  lvar->offset = type_size(type) + already_reserved_offset;

  *lvarsp = lvar;
}

// ========== new node ==========
Node *new_node_op2(NodeKind kind, Node *left, Node *right) {
  if (kind == ND_ADD || kind == ND_SUB) {
    add_type(left);
    add_type(right);
    if (
      left->type->kind == TYPE_PTR
      || left->type->kind == TYPE_ARRAY
    ) {
      right = new_node_op2(ND_MUL, right, new_node_num( type_size(left->type->ptr_to) ));
    } else if (
      right->type->kind == TYPE_PTR
      || right->type->kind == TYPE_ARRAY
    ) {
      left = new_node_op2(ND_MUL, left, new_node_num( type_size(right->type->ptr_to) ));
    }
  }

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

Node *new_node_lvar(char *name, int length, Var *lvars) {
  Var *lvar = find_lvar(name, length, lvars);
  if (!lvar) {
    error("use undeclared identifer '%.*s'", length, name);
  }
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_LVAR;
  node->lvar = lvar;
  return node;
}

Node *new_node_declare_lvar(Type *type, char *name, int length, Var **lvarsp) {
  if (find_lvar(name, length, *lvarsp)!= NULL) {
    error("duplicate declarations '%.*s'", length, name);
  }

  new_lvar(type, name, length, lvarsp);

  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_DECLARE_LVAR;
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
