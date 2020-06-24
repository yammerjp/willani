#include "parse.h"

static Node *new_node_op2(NodeKind kind, Node *left, Node *right, Token *token) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = kind;
  node->left = left;
  node->right = right;
  node->type = type_conversion(node->left->type, node->right->type);
  node->token = token;
  return node;
}

static bool type_is_pointer_or_array(Node *node) {
  return node->type->kind == TYPE_PTR || node->type->kind == TYPE_ARRAY;
}

Node *new_node_add(Node *left, Node *right, Token *token) {
  if (type_is_pointer_or_array(left))
    right = new_node_mul(right, new_node_num( left->type->base->size, left->token ), right->token);
  else if (type_is_pointer_or_array(right))
    left = new_node_mul(left, new_node_num( right->type->base->size, right->token ), left->token);
  return new_node_op2(ND_ADD, left, right, token);
}

Node *new_node_sub(Node *left, Node *right, Token *token) {
  if (type_is_pointer_or_array(left))
    right = new_node_mul(right, new_node_num( left->type->base->size, left->token ), right->token);
  else if (type_is_pointer_or_array(right))
    left = new_node_mul(left, new_node_num( right->type->base->size, right->token ), left->token);
  return new_node_op2(ND_SUB, left, right, token);
}

Node *new_node_mul(Node *left, Node *right, Token *token) {
  return new_node_op2(ND_MUL, left, right, token);
}

Node *new_node_div(Node *left, Node *right, Token *token) {
  return new_node_op2(ND_DIV, left, right, token);
}

Node *new_node_equal(Node *left, Node *right, Token *token) {
  Node *node = new_node_op2(ND_EQ, left, right, token);
  node->type = new_type_int();
  return node;
}

Node *new_node_not_equal(Node *left, Node *right, Token *token) {
  Node *node = new_node_op2(ND_NE, left, right, token);
  node->type = new_type_int();
  return node;
}

Node *new_node_less_than(Node *left, Node *right, Token *token) {
  Node *node = new_node_op2(ND_LT, left, right, token);
  node->type = new_type_int();
  return node;
}

Node *new_node_less_equal(Node *left, Node *right, Token *token) {
  Node *node = new_node_op2(ND_LE, left, right, token);
  node->type = new_type_int();
  return node;
}

Node *new_node_num(long value, Token *token) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_NUM;
  node->value = value;
  node->type = new_type_int();
  node->token = token;
  return node;
}

Node *new_node_string(Token *token) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_STRING;
  node->string = new_string(token->location+1, token->length-2);
  node->type = new_type_array(new_type_char(), node->string->length);
  node->token = token;
  return node;
}

Node *new_node_var(char *name, int length, Token *token) {
  Node *node = calloc(1, sizeof(Node));

  Var *lvar = find_var(name, length, lvars, NULL, INCLUDE_TYPEDEF);
  if (lvar) {
    if (lvar->is_typedef)
      error_at(token->location, "expected a variable but typedef");
    node->kind = ND_LVAR;
    node->var = lvar;
    node->type = lvar->type;
    node->token = token;
    return node;
  }

  Var *gvar = find_var(name, length, gvars, NULL, INCLUDE_TYPEDEF);
  if (gvar) {
    if (gvar->is_typedef)
      error_at(token->location, "expected a variable but typedef");
    node->kind = ND_GVAR;
    node->var = gvar;
    node->type = gvar->type;
    node->token = token;
    return node;
  }

  error_at(name, "use undeclared identifer");
}

Node *new_node_member(Node *parent, char *name, int namelen,  Token *token) {
  Member *member = find_member(parent->type, name, namelen);
  if (!member)
    error_at(token->location, "refered undefined member of struct");
  
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_MEMBER;
  node->left = parent;
  node->type = member->type;
  node->member = member;
  node->token = token;
  return node;
}

Node *new_node_return(Node *left, Token *token) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_RETURN;
  node->left = left;
  node->token = token;
  return node;
}

Node *new_node_if(Node *cond, Node *then, Node *els, Token *token) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_IF;
  node->cond = cond;
  node->then = then;
  node->els = els;
  node->token = token;
  return node;
}

Node *new_node_while(Node *cond, Node *then, Token *token) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_WHILE;
  node->cond = cond;
  node->then = then;
  node->token = token;
  return node;
}

Node *new_node_for(Node *init, Node *cond, Node* increment, Node *then, Token *token) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_FOR;
  node->init = init;
  node->increment = increment;
  node->cond = cond;
  node->then = then;
  node->token = token;
  return node;
}

Node *new_node_block(Node *body, Token *token) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_BLOCK;
  node->body = body;
  node->token = token;
  return node;
}

Node *new_node_func_call(char *name, int len, Node *args, Token *token) {
  Function *func = find_function(name, len);
  if (!func)
    error_at(token->location, "called undefined function");

  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_FUNC_CALL;
  node->func_name = name;
  node->func_args = args;
  node->func_namelen = len;
  node->type = func->type;
  node->token = token;

  return node;
}

Node *new_node_expr_stmt(Node *expr_node, Token *token) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_EXPR_STMT;
  node->left = expr_node;
  node->token = token;
  return node;
}

Node *new_node_addr(Node *unary_node, Token *token) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_ADDR;
  node->left = unary_node;
  node->type = new_type_pointer(unary_node->type);
  node->token = token;
  return node;
}

Node *new_node_deref(Node *unary_node, Token *token) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_DEREF;
  node->left = unary_node;
  node->type = unary_node->type->base;
  node->token = token;
  return node;
}

Node *new_node_assign(Node *left, Node *right, Token *token) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_ASSIGN;
  node->left = left;
  node->right = right;
  node->type = left->type;
  node->token = token;
  return node;
}

Node *new_node_continue(Token *token) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_CONTINUE_STMT;
  node->token = token;
  return node;
}

Node *new_node_break(Token *token) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_BREAK_STMT;
  node->token = token;
  return node;
}

Node *new_node_switch(Node *cond, Node *cases, Node *body, Token *token) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_SWITCH_STMT;
  node->cond = cond;
  node->cases = cases;
  node->body = body;
  node->token = token;
}

Node *new_node_case(Token *token, int case_num) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_CASE_LABEL;
  node->token = token;
  node->case_num = case_num;
  return node;
}

Node *new_node_default(Token *token) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_DEFAULT_LABEL;
  node->token = token;
  return node;
}
