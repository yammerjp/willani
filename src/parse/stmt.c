#include "parse.h"

// block_stmt = "{" stmt* "}"
Node *block_stmt(Token **rest, Token *token) {
  Token *bracket_token = token;
  if (!equal(bracket_token, "{"))
    error_at(bracket_token, "expected {");
  token = bracket_token->next;

  Node head = {};
  Node *tail = &head;
  
  while (!equal(token, "}")) {
    tail->next = stmt(&token, token);
    while (tail->next)
      tail = tail->next;
  }
  token = token->next;

  Node *node = new_node_block(head.next, bracket_token);

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
Node *stmt(Token **rest, Token *token) {
  Node *node;

  Type *type = read_type_tokens(&token, token); // Proceed token if only token means type
  if (type)
    node = declare_lvar_stmt(&token, token, type);
  else if (equal(token, "if"))
    node = if_stmt(&token, token);
  else if (equal(token, "while"))
    node = while_stmt(&token, token);
  else if (equal(token, "for"))
    node = for_stmt(&token, token);
  else if (equal(token, "{"))
    node = block_stmt(&token, token);
  else if (equal(token, "return"))
    node = return_stmt(&token, token);
  else
    node = expr_stmt(&token, token);

  *rest = token;
  return node;
}

// if_stmt = "if" "(" expr ")" stmt ( "else" stmt ) ?
Node *if_stmt(Token **rest, Token *token) {
  Token *if_token = token;
  if (!equal(if_token, "if" ))
    error_at(if_token, "expected if");
  token = if_token->next;

  if (!equal(token, "("))
    error_at(token, "expected (");
  token = token->next;

  Node *cond = expr(&token, token);

  if (!equal(token, ")"))
    error_at(token, "expected )");
  token = token->next;

  Node *then = stmt(&token, token);

  Node *els = NULL;
  if (equal(token, "else")) {
    token = token->next;
    els = stmt(&token, token);
  }
  Node *node = new_node_if(cond, then, els, if_token);

  *rest = token;
  return node;
}

// while_stmt = "while" "(" expr ")" stmt
Node *while_stmt(Token **rest, Token *token) {
  Token *while_token = token;
  if (!equal(while_token, "while" ))
    error_at(while_token, "expected while");
  token = while_token->next;

  if (!equal(token, "("))
    error_at(token, "expected (");
  token = token->next;

  Node *cond = expr(&token, token);

  if (!equal(token, ")"))
    error_at(token, "expected )");
  token = token->next;

  Node *then = stmt(&token, token);

  Node *node = new_node_while(cond, then, while_token);

  *rest = token;
  return node;
}

// for_stmt = "for" "(" expr? ";" expr? ";" expr? ")" stmt
Node *for_stmt(Token **rest, Token *token) {
  // "for"
  Token *for_token = token;
  if (!equal(for_token, "for"))
    error_at(for_token, "expected for");
  token = for_token->next;

  // "("
  if (!equal(token, "("))
    error_at(token, "expected (");
  token = token->next;

  // expr? ";"
  Node *init = NULL;
  if (!equal(token, ";"))
    init = expr(&token, token);

  if (!equal(token, ";"))
    error_at(token, "expected ;");
  token = token->next;

  // expr? ";"
  Node *cond = NULL;
  if (!equal(token, ";"))
    cond = expr(&token, token);

  if (!equal(token, ";"))
    error_at(token, "expected ;");
  token = token->next;

  // expr? ")"
  Node *increment = NULL;
  if (!equal(token, ")"))
    increment = expr(&token, token);

  if (!equal(token, ")"))
    error_at(token, "expected )");
  token = token->next;

  // stmt
  Node *then = stmt(&token, token);
  Node *node = new_node_for(init, cond, increment, then, token);

  *rest = token;
  return node;
}

// return_stmt = return expr ";"
Node *return_stmt(Token **rest, Token *token) {
  Token *return_token = token;
  if (!equal(return_token, "return"))
    error_at(return_token, "expected return");
  token = return_token->next;

  Node *node = new_node_return(expr(&token, token), return_token);

  if (!equal(token, ";"))
    error_at(token, "expected ;");
  token = token->next;

  *rest = token;
  return node;
}

// expr_stmt  =  expr ";"
Node *expr_stmt(Token **rest, Token *token) {
  Token *expr_token = token;
  Node *node = new_node_expr_stmt(expr(&token, token), expr_token);

  if (!equal(token, ";"))
    error_at(token, "expected ;");
  token = token->next;

  *rest = token;
  return node;
}

// declare_lvar_stmt = type identifer type_suffix ("=" expr)? ";"
// type_suffix       = "[" num "]" type_suffix | ε
// declare node is skipped by codegen

Node *declare_lvar_stmt(Token **rest, Token *token, Type *ancestor) {
  // identifer
  if (!is_identifer_token(token))
    error_at(token, "expected identifer");

  char *name = token->location;
  int namelen = token->length;
  token = token->next;

  // ("[" num "]")*
  Type *type = type_suffix(&token, token, ancestor);

  if (find_var(name, namelen, lvars))
    error("duplicate declarations '%.*s'", namelen, name);
  new_var(type, name, namelen, &lvars);

  if (equal(token, ";")) {
    token = token->next;
    *rest = token;
    return NULL;
  }

  if (!equal(token, "="))
    error_at(token, "expected ; or =");
  token = token->next;

  Node *node = init_lvar_stmts(&token, token, NULL);

  if (!equal(token, ";"))
    error_at(token, "expected ;");
  token = token->next;

  *rest = token;
  return node;
}

Type *type_suffix(Token **rest, Token *token, Type *ancestor) {
  if (!equal(token, "["))
    return ancestor;
  token = token->next;

  int length = strtol(token->location, NULL, 10);
  token = token->next;
  if (!equal(token,"]"))
    error_at(token, "expected ]");
  token = token->next;

  Type *parent = type_suffix(&token, token, ancestor);

  *rest = token;
  return new_type_array(parent, length);
}
