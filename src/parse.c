#include "willani.h"

static Node *stmt(Token **rest, Token *token);
static Node *ifstmt(Token **rest, Token *token);
static Node *whilestmt(Token **rest, Token *token);
static Node *expr(Token **rest, Token *token);
static Node *assign(Token **rest, Token *token);
static Node *equality(Token **rest, Token *token);
static Node *relational(Token **rest, Token *token);
static Node *add(Token **rest, Token *token);
static Node *mul(Token **rest, Token *token);
static Node *unary(Token **rest, Token *token);
static Node *primary(Token **rest, Token *token);

// ========== lvar ==========

LVar *locals = NULL;

static LVar *find_lvar(char *name, int length) {
  for (LVar *lvar = locals; lvar; lvar = lvar->next) {
    if (length == lvar->length && !strncmp(name, lvar->name, length)) {
      return lvar;
    }
  }
  return NULL;
}

static LVar *new_lvar(char *name, int length) {
  LVar *lvar = calloc(1, sizeof(LVar));
  lvar->next = locals;
  lvar->name = name;
  lvar->length = length;
  lvar->offset = locals ? (locals->offset + 8) : 8;

  locals = lvar;
  return lvar;
}

// ========== new node ==========
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

static Node *new_node_var(char *name, int length) {
  LVar *lvar = find_lvar(name, length);
  if (!lvar) {
    lvar = new_lvar(name, length);
  }
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_VAR;
  node->lvar = lvar;
  return node;
}

static Node *new_node_return(Node *left) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_RETURN;
  node->left = left;
  return node;
}

static Node *new_node_if(Node *cond, Node *then, Node *els) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_IF;
  node->cond = cond;
  node->then = then;
  node->els = els;
  return node;
}

static Node *new_node_while(Node *cond, Node *then) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_WHILE;
  node->cond = cond;
  node->then = then;
  return node;
}



// ========== parse ==========

// program = stmt*
Function *program(Token *token) {
  Node head = {};
  Node *current = &head;

  while (!is_eof_token(token)) {
    current->next = stmt(&token, token);
    current = current->next;
  }

  Function *func = calloc(1, sizeof(Function));
  func->node = head.next;
  func->lvar = locals;

  return func;
}

// stmt       = ifstmt
//            | whilestmt
//            | (return)? expr ";
static Node *stmt(Token **rest, Token *token) {
  Node *node;
  if (equal(token, "if")) {
    node = ifstmt(&token, token);
    *rest = token;
    return node;
  }
  if (equal(token, "while")) {
    node = whilestmt(&token, token);
    *rest = token;
    return node;
  }

  if (equal(token, "return")) {
    token = token->next;
    node = new_node_return(expr(&token, token));
  } else {
    node = expr(&token, token);
  }

  if (!equal(token, ";")) {
    error_at(token, "expected ;");
  }
  token = token->next;

  *rest = token;
  return node;
}

// ifstmt = "if" "(" expr ")" stmt ( "else" stmt ) ?
static Node *ifstmt(Token **rest, Token *token) {
  if (!equal(token, "if" )) {
    error_at(token, "expected if");
  }
  token = token->next;
  if (!equal(token, "(")) {
    error_at(token, "expected (");
  }
  token = token->next;

  Node *cond = expr(&token, token);

  if (!equal(token, ")")) {
    error_at(token, "expected )");
  }
  token = token->next;

  Node *then = stmt(&token, token);

  Node *els = NULL;
  if (equal(token, "else")) {
    token = token->next;
    els = stmt(&token, token);
  }
  Node *node = new_node_if(cond, then, els);

  *rest = token;
  return node;
}

// whilestmt = "while" "(" expr ")" stmt
static Node *whilestmt(Token **rest, Token *token) {
  if (!equal(token, "while" )) {
    error_at(token, "expected while");
  }
  token = token->next;
  if (!equal(token, "(")) {
    error_at(token, "expected (");
  }
  token = token->next;

  Node *cond = expr(&token, token);

  if (!equal(token, ")")) {
    error_at(token, "expected )");
  }
  token = token->next;

  Node *then = stmt(&token, token);

  Node *node = new_node_while(cond, then);

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
    node = new_node_op2( ND_ASSIGN, node, assign(&token, token));
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

// primary    = num | ident | "(" expr ")"
static Node *primary(Token **rest, Token *token) {
  if (is_number_token(token)) {
    Node *node = new_node_num(strtol(token->location, NULL, 10));
    token = token->next;
    *rest = token;
    return node;
  }

  if (is_identifer_token(token)) {
    Node *node = new_node_var(token->location, token->length);
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
