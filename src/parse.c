#include "willani.h"

static Function *function(Token **rest, Token *token);
static Node *stmt(Token **rest, Token *token, LVar **lvarsp);
static Node *if_stmt(Token **rest, Token *token, LVar **lvarsp);
static Node *while_stmt(Token **rest, Token *token, LVar **lvarsp);
static Node *for_stmt(Token **rest, Token *token, LVar **lvarsp);
static Node *block_stmt(Token **rest, Token *token, LVar **lvarsp);
static Node *expr_stmt(Token **rest, Token *token, LVar **lvarsp);
static Node *return_stmt(Token **rest, Token *token, LVar **lvarsp);
static Node *expr(Token **rest, Token *token, LVar **lvarsp);
static Node *assign(Token **rest, Token *token, LVar **lvarsp);
static Node *equality(Token **rest, Token *token, LVar **lvarsp);
static Node *relational(Token **rest, Token *token, LVar **lvarsp);
static Node *add(Token **rest, Token *token, LVar **lvarsp);
static Node *mul(Token **rest, Token *token, LVar **lvarsp);
static Node *unary(Token **rest, Token *token, LVar **lvarsp);
static Node *primary(Token **rest, Token *token, LVar **lvarsp);

// ========== lvar ==========

static LVar *find_lvar(char *name, int length, LVar *lvars) {
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

static Node *new_node_var(char *name, int length, LVar **lvarsp) {
  LVar *lvar = find_lvar(name, length, *lvarsp);
  if (!lvar) {
    new_lvar(name, length, lvarsp);
    lvar = *lvarsp;
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

static Node *new_node_for(Node *init, Node *cond, Node* increment, Node *then) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_FOR;
  node->init = init;
  node->increment = increment;
  node->cond = cond;
  node->then = then;
  return node;
}

static Node *new_node_block(Node *body) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_BLOCK;
  node->body = body;
  return node;
}

static Node *new_node_func_call(char *name, int len, Node *args) {
  FuncCall *fncl = calloc(1, sizeof(FuncCall));
  fncl->name = name;
  fncl->args = args;
  fncl->length = len;
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_FUNC_CALL;
  node->fncl = fncl;
  return node;
}

static Node *new_node_expr_stmt(Node *stmt_node) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_EXPR_STMT;
  node->left = stmt_node;
  return node;
}



// ========== parse ==========

// program = function*
Function *program(Token *token) {

  LVar *lvars = NULL;
  Function head = {};
  Function *current = &head;

  while (!is_eof_token(token)) {
    current->next = function(&token, token);
    current = current->next;
  }

  return head.next;
}

// function = ( ident "(" ( ( ident ( "," ident ) * ) ?  ")" block_stmt ) *

Function *function(Token **rest, Token *token) {
  LVar *lvars = NULL;
  if (!is_identifer_token(token)) {
    error_at(token, "expected identifer");
  }
  char *name = token->location;
  int length = token->length;
  token = token->next;

  int argc = 0;
  if (!equal(token, "(")) {
    error_at(token, "expected (");
  }
  token = token->next;

  while (is_identifer_token(token)) {
    argc ++;
    new_lvar(token->location, token->length, &lvars);
    token = token->next;
    if (!equal(token, ",")) {
      break;
    }
    token = token->next;
  }
  if (!equal(token, ")")) {
    error_at(token, "expected )");
  }
  token = token->next;

  Node *node = block_stmt(&token, token, &lvars);

  Function *func = calloc(1, sizeof(Function));
  func->node = node;
  func->lvar = lvars;
  func->name = name;
  func->argc = argc;
  func->namelen = length;

  *rest = token;
  return func;
}

// block_stmt = "{" stmt* "}"
static Node *block_stmt(Token **rest, Token *token, LVar **lvarsp) {
  if (!equal(token, "{")) {
    error_at(token, "expected {");
  }
  token = token->next;

  Node head = {};
  Node *current = &head;
  
  while (!equal(token, "}")) {
    current->next = stmt(&token, token, lvarsp);
    current = current->next;
  }
  token = token->next;

  Node *node = new_node_block(head.next);

  *rest = token;
  return node;
}


// stmt       = block_stmt
//            | if_stmt
//            | while_stmt
//            | for_stmt
//            | return_stmt
//            | expr_stmt
static Node *stmt(Token **rest, Token *token, LVar **lvarsp) {
  Node *node;
  if (equal(token, "if")) {
    node = if_stmt(&token, token, lvarsp);
    *rest = token;
    return node;
  }
  if (equal(token, "while")) {
    node = while_stmt(&token, token, lvarsp);
    *rest = token;
    return node;
  }
  if (equal(token, "for")) {
    node = for_stmt(&token, token, lvarsp);
    *rest = token;
    return node;
  }
  if (equal(token, "{")) {
    node = block_stmt(&token, token, lvarsp);
    *rest = token;
    return node;
  }

  if (equal(token, "return")) {
    node = return_stmt(&token, token, lvarsp);
    *rest = token;
    return node;
  }

  node = expr_stmt(&token, token, lvarsp);
  *rest = token;
  return node;
}

// if_stmt = "if" "(" expr ")" stmt ( "else" stmt ) ?
static Node *if_stmt(Token **rest, Token *token, LVar **lvarsp) {
  if (!equal(token, "if" )) {
    error_at(token, "expected if");
  }
  token = token->next;
  if (!equal(token, "(")) {
    error_at(token, "expected (");
  }
  token = token->next;

  Node *cond = expr(&token, token, lvarsp);

  if (!equal(token, ")")) {
    error_at(token, "expected )");
  }
  token = token->next;

  Node *then = stmt(&token, token, lvarsp);

  Node *els = NULL;
  if (equal(token, "else")) {
    token = token->next;
    els = stmt(&token, token, lvarsp);
  }
  Node *node = new_node_if(cond, then, els);

  *rest = token;
  return node;
}

// while_stmt = "while" "(" expr ")" stmt
static Node *while_stmt(Token **rest, Token *token, LVar **lvarsp) {
  if (!equal(token, "while" )) {
    error_at(token, "expected while");
  }
  token = token->next;
  if (!equal(token, "(")) {
    error_at(token, "expected (");
  }
  token = token->next;

  Node *cond = expr(&token, token, lvarsp);

  if (!equal(token, ")")) {
    error_at(token, "expected )");
  }
  token = token->next;

  Node *then = stmt(&token, token, lvarsp);

  Node *node = new_node_while(cond, then);

  *rest = token;
  return node;
}

// for_stmt = "for" "(" expr? ";" expr? ";" expr? ")" stmt
static Node *for_stmt(Token **rest, Token *token, LVar **lvarsp) {
  // "for"
  if (!equal(token, "for")) {
    error_at(token, "expected for");
  }
  token = token->next;

  // "("
  if (!equal(token, "(")) {
    error_at(token, "expected (");
  }
  token = token->next;

  // expr? ";"
  Node *init = NULL;
  if (!equal(token, ";")) {
    init = expr(&token, token, lvarsp);
  }
  if (!equal(token, ";")) {
    error_at(token, "expected ;");
  }
  token = token->next;

  // expr? ";"
  Node *cond = NULL;
  if (!equal(token, ";")) {
    cond = expr(&token, token, lvarsp);
  }
  if (!equal(token, ";")) {
    error_at(token, "expected ;");
  }
  token = token->next;

  // expr? ")"
  Node *increment = NULL;
  if (!equal(token, ")")) {
    increment = expr(&token, token, lvarsp);
  }
  if (!equal(token, ")")) {
    error_at(token, "expected )");
  }
  token = token->next;

  // stmt
  Node *then = stmt(&token, token, lvarsp);
  Node *node = new_node_for(init, cond, increment, then);

  *rest = token;
  return node;
}

// return_stmt = return expr ";"
static Node *return_stmt(Token **rest, Token *token, LVar **lvarsp) {
  if (!equal(token, "return")) {
    error_at(token, "expected return");
  }
  token = token->next;

  Node *node = new_node_return(expr(&token, token, lvarsp));

  if (!equal(token, ";")) {
    error_at(token, "expected ;");
  }
  token = token->next;

  *rest = token;
  return node;
}

// expr_stmt  =  expr ";"
static Node *expr_stmt(Token **rest, Token *token, LVar **lvarsp) {
  Node *node = new_node_expr_stmt(expr(&token, token, lvarsp));

  if (!equal(token, ";")) {
    error_at(token, "expected ;");
  }
  token = token->next;

  *rest = token;
  return node;
}

// expr       = assign
static Node *expr(Token **rest, Token *token, LVar **lvarsp) {
  Node *node = assign(&token, token, lvarsp);
  *rest = token;
  return node;
}

// assign     = equality ("=" assign)?
static Node *assign(Token **rest, Token *token, LVar **lvarsp) {
  Node *node = equality(&token, token, lvarsp);

  if(equal(token, "=")) {
    token = token->next;
    node = new_node_op2( ND_ASSIGN, node, assign(&token, token, lvarsp));
  }

  *rest = token;
  return node;
}

// equality = relational ("==" relational | "!=" relational)*
static Node *equality(Token **rest, Token *token, LVar **lvarsp) {
  Node *node = relational(&token, token, lvarsp);
  for(;;) {
    if (equal(token, "==")){
      token = token->next;
      node = new_node_op2(ND_EQ, node, relational(&token, token, lvarsp));
    } else if (equal(token, "!=")) {
      token = token->next;
      node = new_node_op2(ND_NE, node, relational(&token, token, lvarsp));
    } else {
      *rest = token;
      return node;
    }
  }
}

// relational = add ("<" add | "<=" add | ">" add | ">=" add)*
static Node *relational(Token **rest, Token *token, LVar ** lvarsp) {
  Node *node = add(&token, token, lvarsp);
  for(;;) {
    if (equal(token, "<")){
      token = token->next;
      node = new_node_op2(ND_LT, node, add(&token, token, lvarsp));
      continue;
    }
    if (equal(token, "<=")) {
      token = token->next;
      node = new_node_op2(ND_LE, node, add(&token, token, lvarsp));
      continue;
    }
    if (equal(token, ">")){
      token = token->next;
      node = new_node_op2(ND_LT, add(&token, token, lvarsp), node);
      continue;
    }
    if (equal(token, ">=")) {
      token = token->next;
      node = new_node_op2(ND_LE, add(&token, token, lvarsp), node);
      continue;
    }
    *rest = token;
    return node;
  }
}

// add = mul ("+" mul | "-" mul)*
static Node *add(Token **rest, Token *token, LVar **lvarsp) {
  Node *node = mul(&token, token, lvarsp);
  for(;;) {
    if (equal(token, "+")){
      token = token->next;
      node = new_node_op2(ND_ADD, node, mul(&token, token, lvarsp));
    } else if (equal(token, "-")) {
      token = token->next;
      node = new_node_op2(ND_SUB, node, mul(&token, token, lvarsp));
    } else {
      *rest = token;
      return node;
    }
  }
}

// mul = unary ("*" unary | "/" unary)*
static Node *mul(Token **rest, Token *token, LVar **lvarsp) {
  Node *node = unary(&token, token, lvarsp);

  for(;;) {
    if (equal(token, "*")) {
      token = token->next;
      node = new_node_op2(ND_MUL, node, unary(&token, token, lvarsp));
    } else if (equal(token, "/")) {
      token = token->next;
      node = new_node_op2(ND_DIV, node, unary(&token, token, lvarsp));
    } else {
      *rest = token;
      return node;
    }
  }
}

// unary = ("+" | "-")? primary
static Node *unary(Token **rest, Token *token, LVar **lvarsp) {
  if (equal(token,"+")) {
    token = token->next;
    Node *node = primary(&token, token, lvarsp);
    *rest = token;
    return node;
  }
  if (equal(token,"-")) {
    token = token->next;
    Node *node = new_node_op2(ND_SUB, new_node_num(0), primary(&token, token, lvarsp));
    *rest = token;
    return node;
  }
  Node *node = primary(&token, token, lvarsp);
  *rest = token;
  return node;
}

// primary    = num | ident ( "(" ")" )? | "(" expr ")"
static Node *primary(Token **rest, Token *token, LVar **lvarsp) {
  Node *node;
  if (is_number_token(token)) {
    node = new_node_num(strtol(token->location, NULL, 10));
    token = token->next;
    *rest = token;
    return node;
  }

  if (is_identifer_token(token)) {

    if (!equal(token->next, "(")) {
      node = new_node_var(token->location, token->length, lvarsp);
      token = token->next;
    } else {
      char *name = token->location;
      int length = token->length;
      token = token->next->next;
      Node args_head = {};
      Node *args_current = &args_head;
      for(int i=0;i<6;i++) {
        if (equal(token, ")")) {
          break;
        }
        args_current->next = expr(&token, token, lvarsp);
        args_current = args_current->next;
        if (!equal(token, ",")) {
          break;
        }
        token = token->next;
      }
      if (!equal(token, ")")) {
        error_at(token, "expected )");
      }
      token = token->next;
      node = new_node_func_call(name, length, args_head.next);
    }
    *rest = token;
    return node;
  }

  if (!equal(token,"(")) {
    error_at(token, "expected (");
  }

  token = token->next;
  node = expr(&token, token, lvarsp);

  if (!equal(token,")")) {
    error_at(token, "expected )");
  }
  token = token->next;
  *rest = token;
  return node;
}
