#include "willani.h"

static Node *stmt(Token **rest, Token *token);
static Node *expr(Token **rest, Token *token);
static Node *assign(Token **rest, Token *token);
static Node *equality(Token **rest, Token *token);
static Node *relational(Token **rest, Token *token);
static Node *add(Token **rest, Token *token);
static Node *mul(Token **rest, Token *token);
static Node *unary(Token **rest, Token *token);
static Node *primary(Token **rest, Token *token);

static Node *new_node_op2(Token *token, NodeKind kind, Node *left, Node *right) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = kind;
  node->next = NULL;
  node->left = left;
  node->right = right;
  node->value = 0;
  node->lvar = NULL;
  node->token = token; // for debug

  return node;
}

static Node *new_node_num(long value, Token *token) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_NUM;
  node->next = NULL;
  node->left = NULL;
  node->right = NULL;
  node->value = value;
  node->lvar = NULL;
  node->token = token;

  return node;
}

LVar *locals = NULL;

static LVar *find_lvar(Token *token) {
  for (LVar *var = locals; var; var = var->next) {
    if (var->length == token->length && strncmp(token->location, var->name, token->length)) {
      return var;
    }
  }
  return NULL;
}

static LVar *new_lvar(char *name, int length) {
  LVar *lvar = calloc(1, sizeof(LVar));
  lvar->next = locals;
  lvar->name = name;
  lvar->length = length;
  return lvar;
}

static Node *new_node_var(Token *token) {
  LVar *lvar = find_lvar(token);
  if (lvar==NULL) {
    // create new lvar
    lvar = new_lvar(token->location, token->length);
  }

  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_VAR;
  node->next = NULL;
  node->left = NULL;
  node->right = NULL;
  node->value = 0;
  node->lvar = lvar;
  node->token = token;

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

// stmt       = expr ";"
static Node *stmt(Token **rest, Token *token) {
  Node *node = expr(&token, token);
  if (!equal(token, ";")) {
    error_at(token, "expected ;");
  }
  token = token->next;

  *rest = token;
  return node;
}

// expr       = assign
static Node *expr(Token **rest, Token *token) {
  Node *node = assign(&token, token);
  *rest = token;
  return node;
}

// assign     = equality ("=" assign)?
static Node *assign(Token **rest, Token *token) {
  Node *node = equality(&token, token);

  if(equal(token, "=")) {
    token = token->next;
    node = new_node_op2(token, ND_ASSIGN, node, assign(&token, token));
  }

  *rest = token;
  return node;
}

// equality = relational ("==" relational | "!=" relational)*
static Node *equality(Token **rest, Token *token) {
  Node *node = relational(&token, token);
  for(;;) {
    if (equal(token, "==")){
      token = token->next;
      node = new_node_op2(token, ND_EQ, node, relational(&token, token));
    } else if (equal(token, "!=")) {
      token = token->next;
      node = new_node_op2(token, ND_NE, node, relational(&token, token));
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
      node = new_node_op2(token, ND_LT, node, add(&token, token));
      continue;
    }
    if (equal(token, "<=")) {
      token = token->next;
      node = new_node_op2(token, ND_LE, node, add(&token, token));
      continue;
    }
    if (equal(token, ">")){
      token = token->next;
      node = new_node_op2(token, ND_LT, add(&token, token), node);
      continue;
    }
    if (equal(token, ">=")) {
      token = token->next;
      node = new_node_op2(token, ND_LE, add(&token, token), node);
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
      node = new_node_op2(token, ND_ADD, node, mul(&token, token));
    } else if (equal(token, "-")) {
      token = token->next;
      node = new_node_op2(token, ND_SUB, node, mul(&token, token));
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
      node = new_node_op2(token, ND_MUL, node, unary(&token, token));
    } else if (equal(token, "/")) {
      token = token->next;
      node = new_node_op2(token, ND_DIV, node, unary(&token, token));
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
    Node *node = new_node_op2(token, ND_SUB, new_node_num(0, token), primary(&token, token));
    *rest = token;
    return node;
  }
  Node *node = primary(&token, token);
  *rest = token;
  return node;
}

// primary    = num | ident | "(" expr ")"
static Node *primary(Token **rest, Token *token) {
  if (is_number_token(token)) {
    Node *node = new_node_num(get_number(token), token);
    token = token->next;
    *rest = token;
    return node;
  }

  if (is_identifer_token(token)) {
    Node *node = new_node_var(token);
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
