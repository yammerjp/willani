#include "parse.h"

// block_stmt = "{" stmt* "}"
Node *block_stmt(Token **rest, Token *token, Var **lvarsp) {
  if (!equal(token, "{")) {
    error_at_token(token, "expected {");
  }
  token = token->next;

  Node head = {};
  Node *tail = &head;
  
  while (!equal(token, "}")) {
    tail->next = stmt(&token, token, lvarsp);
    while (tail->next) {
      tail = tail->next;
    }
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
//            | declare_lvar_stmt
Node *stmt(Token **rest, Token *token, Var **lvarsp) {
  Node *node;

  Type *type = read_type_tokens(&token, token); // Proceed token if only token means type
  if (type) {
    node = declare_lvar_stmt(&token, token, lvarsp, type);
  } else if (equal(token, "if")) {
    node = if_stmt(&token, token, lvarsp);
  } else if (equal(token, "while")) {
    node = while_stmt(&token, token, lvarsp);
  } else if (equal(token, "for")) {
    node = for_stmt(&token, token, lvarsp);
  } else if (equal(token, "{")) {
    node = block_stmt(&token, token, lvarsp);
  } else if (equal(token, "return")) {
    node = return_stmt(&token, token, lvarsp);
  } else {
    node = expr_stmt(&token, token, lvarsp);
  }

  *rest = token;
  return node;
}

// if_stmt = "if" "(" expr ")" stmt ( "else" stmt ) ?
Node *if_stmt(Token **rest, Token *token, Var **lvarsp) {
  if (!equal(token, "if" )) {
    error_at_token(token, "expected if");
  }
  token = token->next;
  if (!equal(token, "(")) {
    error_at_token(token, "expected (");
  }
  token = token->next;

  Node *cond = expr(&token, token, lvarsp);

  if (!equal(token, ")")) {
    error_at_token(token, "expected )");
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
Node *while_stmt(Token **rest, Token *token, Var **lvarsp) {
  if (!equal(token, "while" )) {
    error_at_token(token, "expected while");
  }
  token = token->next;
  if (!equal(token, "(")) {
    error_at_token(token, "expected (");
  }
  token = token->next;

  Node *cond = expr(&token, token, lvarsp);

  if (!equal(token, ")")) {
    error_at_token(token, "expected )");
  }
  token = token->next;

  Node *then = stmt(&token, token, lvarsp);

  Node *node = new_node_while(cond, then);

  *rest = token;
  return node;
}

// for_stmt = "for" "(" expr? ";" expr? ";" expr? ")" stmt
Node *for_stmt(Token **rest, Token *token, Var **lvarsp) {
  // "for"
  if (!equal(token, "for")) {
    error_at_token(token, "expected for");
  }
  token = token->next;

  // "("
  if (!equal(token, "(")) {
    error_at_token(token, "expected (");
  }
  token = token->next;

  // expr? ";"
  Node *init = NULL;
  if (!equal(token, ";")) {
    init = expr(&token, token, lvarsp);
  }
  if (!equal(token, ";")) {
    error_at_token(token, "expected ;");
  }
  token = token->next;

  // expr? ";"
  Node *cond = NULL;
  if (!equal(token, ";")) {
    cond = expr(&token, token, lvarsp);
  }
  if (!equal(token, ";")) {
    error_at_token(token, "expected ;");
  }
  token = token->next;

  // expr? ")"
  Node *increment = NULL;
  if (!equal(token, ")")) {
    increment = expr(&token, token, lvarsp);
  }
  if (!equal(token, ")")) {
    error_at_token(token, "expected )");
  }
  token = token->next;

  // stmt
  Node *then = stmt(&token, token, lvarsp);
  Node *node = new_node_for(init, cond, increment, then);

  *rest = token;
  return node;
}

// return_stmt = return expr ";"
Node *return_stmt(Token **rest, Token *token, Var **lvarsp) {
  if (!equal(token, "return")) {
    error_at_token(token, "expected return");
  }
  token = token->next;

  Node *node = new_node_return(expr(&token, token, lvarsp));

  if (!equal(token, ";")) {
    error_at_token(token, "expected ;");
  }
  token = token->next;

  *rest = token;
  return node;
}

// expr_stmt  =  expr ";"
Node *expr_stmt(Token **rest, Token *token, Var **lvarsp) {
  Node *node = new_node_expr_stmt(expr(&token, token, lvarsp));

  if (!equal(token, ";")) {
    error_at_token(token, "expected ;");
  }
  token = token->next;

  *rest = token;
  return node;
}

// declare_lvar_stmt = type identifer type_suffix ("=" expr)? ";"
// type_suffix       = "[" num "]" type_suffix | ε
// declare node is skipped by codegen

Node *declare_lvar_stmt(Token **rest, Token *token, Var **lvarsp, Type *ancestor) {
  // identifer
  if (!is_identifer_token(token)) {
    error_at_token(token, "expected identifer");
  }
  char *name = token->location;
  int namelen = token->length;
  token = token->next;

  // ("[" num "]")*
  Type *type = type_suffix(&token, token, ancestor);

  if (find_var(name, namelen, *lvarsp)) {
    error("duplicate declarations '%.*s'", namelen, name);
  }
  new_var(type, name, namelen, lvarsp);

  if (equal(token, ";")) {
    token = token->next;
    *rest = token;
    return NULL;
  }

  if (!equal(token, "=")) {
    error_at_token(token, "expected ; or =");
  }
  token = token->next;

  Node *node = init_lvar_stmts(&token, token, lvarsp, NULL);

  if (!equal(token, ";")) {
    error_at_token(token, "expected ;");
  }
  token = token->next;

  *rest = token;
  return node;
}

Type *type_suffix(Token **rest, Token *token, Type *ancestor) {
  if (!equal(token, "[")) {
    return ancestor;
  }
  token = token->next;

  int length = strtol(token->location, NULL, 10);
  token = token->next;
  if (!equal(token,"]")) {
    error_at_token(token, "expected ]");
  }
  token = token->next;

  Type *parent = type_suffix(&token, token, ancestor);

  *rest = token;
  return new_type_array(parent, length);
}
