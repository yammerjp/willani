#include "willani.h"

// ========== var ==========
Var *find_var(char *name, int length, Var *vars) {
  for (Var *var = vars; var; var = var->next) {
    if (length == var->length && !strncmp(name, var->name, length)) {
      return var;
    }
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

Node *new_node_var(char *name, int length, Var *lvars) {
  Node *node = calloc(1, sizeof(Node));

  Var *gvar = find_var(name, length, gvars);
  if (gvar) {
    node->kind = ND_GVAR;
    node->var = gvar;
    return node;
  }

  Var *lvar = find_var(name, length, lvars);
  if (lvar) {
    node->kind = ND_LVAR;
    node->var = lvar;
    return node;
  }

  error("use undeclared identifer '%.*s'", length, name);
}

Node *new_node_declare_lvar(Type *type, char *name, int length, Var **lvarsp) {
  if (find_var(name, length, *lvarsp)!= NULL) {
    error("duplicate declarations '%.*s'", length, name);
  }

  new_var(type, name, length, lvarsp);

  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_DECLARE_LVAR;
  node->var = *lvarsp;
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
