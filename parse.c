#include "willani.h"

static Node *stmt(Token **rest, Token *token);
static Node *expr(Token **rest, Token *token);
static Node *relational(Token **rest, Token *token);
static Node *add(Token **rest, Token *token);
static Node *mul(Token **rest, Token *token);
static Node *unary(Token **rest, Token *token);
static Node *primary(Token **rest, Token *token);

static Node *new_node_op2(NodeKind kind, Node *left, Node *right) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = kind;
  node->left = left;
  node->right = right;
  return node;
}

static Node *new_node_num(long value) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_NUM;
  node->value = value;
  return node;
}

// program = stmt*
Node *program(Token **rest, Token *token) {
  Node head = {};
  Node *current = &head;

  while (token->kind != TK_EOF) {
    current->next = stmt(&token, token);
    current = current->next;
 }

  *rest = token;
  return head.next;
}

Node *stmt(Token **rest, Token *token) {
  Node *node = expr(&token, token);
  if (!equal(token, ";")) {
    error_at(token, "expected ;");
  }
  token = token->next;

  *rest = token;
  return node;
}

// expr     = equality
// equality = relational ("==" relational | "!=" relational)*
Node *expr(Token **rest, Token *token) {
  Node *node = relational(&token, token);
  for(;;) {
    if (equal(token, "==")){
      token = token->next;
      node = new_node_op2(ND_EQ, node, relational(&token, token));
    } else if (equal(token, "!=")) {
      token = token->next;
      node = new_node_op2(ND_NE, node, relational(&token, token));
    } else {
      *rest = token;
      return node;
    }
  }
}

// relational = add ("<" add | "<=" add | ">" add | ">=" add)*
static Node *relational(Token **rest, Token *token) {
  Node *node = add(&token, token);
  for(;;) {
    if (equal(token, "<")){
      token = token->next;
      node = new_node_op2(ND_LT, node, add(&token, token));
      continue;
    }
    if (equal(token, "<=")) {
      token = token->next;
      node = new_node_op2(ND_LE, node, add(&token, token));
      continue;
    }
    if (equal(token, ">")){
      token = token->next;
      node = new_node_op2(ND_LT, add(&token, token), node);
      continue;
    }
    if (equal(token, ">=")) {
      token = token->next;
      node = new_node_op2(ND_LE, add(&token, token), node);
      continue;
    }
    *rest = token;
    return node;
  }
}

// add = mul ("+" mul | "-" mul)*
static Node *add(Token **rest, Token *token) {
  Node *node = mul(&token, token);
  for(;;) {
    if (equal(token, "+")){
      token = token->next;
      node = new_node_op2(ND_ADD, node, mul(&token, token));
    } else if (equal(token, "-")) {
      token = token->next;
      node = new_node_op2(ND_SUB, node, mul(&token, token));
    } else {
      *rest = token;
      return node;
    }
  }
}

// mul = unary ("*" unary | "/" unary)*
static Node *mul(Token **rest, Token *token) {
  Node *node = unary(&token, token);

  for(;;) {
    if (equal(token, "*")) {
      token = token->next;
      node = new_node_op2(ND_MUL, node, unary(&token, token));
    } else if (equal(token, "/")) {
      token = token->next;
      node = new_node_op2(ND_DIV, node, unary(&token, token));
    } else {
      *rest = token;
      return node;
    }
  }
}

// unary = ("+" | "-")? primary
static Node *unary(Token **rest, Token *token) {
  if (equal(token,"+")) {
    token = token->next;
    Node *node = primary(&token, token);
    *rest = token;
    return node;
  }
  if (equal(token,"-")) {
    token = token->next;
    Node *node = new_node_op2(ND_SUB, new_node_num(0), primary(&token, token));
    *rest = token;
    return node;
  }
  Node *node = primary(&token, token);
  *rest = token;
  return node;
}

// primary = num | "(" expr ")"
static Node *primary(Token **rest, Token *token) {
  if (is_number_token(token)) {
    Node *node = new_node_num(get_number(token));
    token = token->next;
    *rest = token;
    return node;
  }

  if (!equal(token,"(")) {
    error_at(token, "expected (");
  }

  token = token->next;
  Node *node = expr(&token, token);

  if (!equal(token,")")) {
    error_at(token, "expected )");
  }
  token = token->next;
  *rest = token;
  return node;
}
