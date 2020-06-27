#include "parse.h"

// block_stmt = "{" stmt* "}"
Node *block_stmt(Token **rest, Token *token) {
  Token *bracket_token = token;
  if (!equal(bracket_token, "{"))
    error_at(bracket_token->location, "expected {");
  token = bracket_token->next;

  Node head = {};
  Node *tail = &head;
  while (!equal(token, "}")) {
    tail->next = stmt(&token, token);
    while (tail->next)
      tail = tail->next;
  }

  *rest = token->next;
  return new_node_block(head.next, bracket_token);
}


// stmt       = block_stmt
//            | if_stmt
//            | while_stmt
//            | for_stmt
//            | return_stmt
//            | expr_stmt
//            | "continue" ";"
//            | "break" ";"
//            | switch_stmt
//            | "case" expr ":"
//            | "default" ":"

//            | declare_lvar_stmt

Node *stmt(Token **rest, Token *token) {
  Node *node;

  if (equal(token, "typedef")) {
    typedef_stmt(&token, token);
    *rest = token;
    return NULL;
  }

  Type *type = read_type(&token, token, ALLOW_STATIC); // Proceed token if only token means type
  if (type)
    node = declare_lvar_stmt(&token, token, type);
  else {
    scope_in();
    node = stmt_without_declaration(&token, token);
    scope_out();
  }
  *rest = token;
  return node;
}

Node *stmt_without_declaration(Token **rest, Token *token) {
  if (read_type(&token, token, DENY_STATIC))
    error_at(token->location, "declaration statement is invalid here");

  Node *node;

  if (equal(token, "if"))
    node = if_stmt(&token, token);
  else if (equal(token, "while"))
    node = while_stmt(&token, token);
  else if (equal(token, "for")) {
    scope_in();
    node = for_stmt(&token, token);
    scope_out();
  }
  else if (equal(token, "switch"))
    node = switch_stmt(&token, token);
  else if (equal(token, "{"))
    node = block_stmt(&token, token);
  else if (equal(token, "return"))
    node = return_stmt(&token, token);
  else if (equal(token, "continue")) {
    if (!equal(token->next, ";"))
      error_at(token->next->location, "expected ';' of continue statement");
    node = new_node_continue(token);
    token = token->next->next;
  } else if (equal(token, "break")) {
    if (!equal(token->next, ";"))
      error_at(token->next->location, "expected ';' of break statement");
    node = new_node_break(token);
    token = token->next->next;
  } else
    node = expr_stmt(&token, token);

  *rest = token;
  return node;
}


// if_stmt = "if" "(" expr ")" stmt ( "else" stmt ) ?
Node *if_stmt(Token **rest, Token *token) {
  Token *if_token = token;
  if (!equal(if_token, "if" ))
    error_at(if_token->location, "expected if");
  token = if_token->next;

  if (!equal(token, "("))
    error_at(token->location, "expected (");
  token = token->next;

  Node *cond = expr(&token, token);

  if (!equal(token, ")"))
    error_at(token->location, "expected )");
  token = token->next;

  scope_in();
  Node *then = stmt_without_declaration(&token, token);
  scope_out();

  Node *els = NULL;
  if (equal(token, "else")) {
    token = token->next;
    scope_in();
    els = stmt_without_declaration(&token, token);
    scope_out();
  }
  Node *node = new_node_if(cond, then, els, if_token);

  *rest = token;
  return node;
}

// while_stmt = "while" "(" expr ")" stmt
Node *while_stmt(Token **rest, Token *token) {
  Token *while_token = token;
  if (!equal(while_token, "while" ))
    error_at(while_token->location, "expected while");
  token = while_token->next;

  if (!equal(token, "("))
    error_at(token->location, "expected (");
  token = token->next;

  Node *cond = expr(&token, token);

  if (!equal(token, ")"))
    error_at(token->location, "expected )");
  token = token->next;

  scope_in();
  Node *then = stmt_without_declaration(&token, token);
  scope_out();

  Node *node = new_node_while(cond, then, while_token);

  *rest = token;
  return node;
}

Node *for_stmt(Token **rest, Token *token) {
  // "for"
  Token *for_token = token;
  if (!equal(for_token, "for"))
    error_at(for_token->location, "expected for");
  token = for_token->next;

  // "("
  if (!equal(token, "("))
    error_at(token->location, "expected (");
  token = token->next;

  // expr? ";"
  Node *init = NULL;
  if (!equal(token, ";")) {
    Type *type = read_type(&token, token, ALLOW_STATIC);
    if (type) {
      init = declare_lvar_stmt(&token, token, type);
    } else {
      Token *expr_token = token;
      init = new_node_expr_stmt(expr(&token, token), expr_token);

      if (!equal(token, ";"))
        error_at(token->location, "expected ;");
      token = token->next;
    }
  } else {
    token = token->next;
  }

    // expr? ";"
  Node *cond = NULL;
  if (!equal(token, ";"))
    cond = expr(&token, token);
  else
    cond = new_node_num(1, token);

  if (!equal(token, ";"))
    error_at(token->location, "expected ;");
  token = token->next;

  // expr? ")"
  Node *increment = NULL;
  if (!equal(token, ")")) {
    Token *expr_token = token;
    increment = new_node_expr_stmt(expr(&token, token), expr_token);
  }

  if (!equal(token, ")"))
    error_at(token->location, "expected )");
  token = token->next;

  Node *then = stmt_without_declaration(&token, token);
  Node *node = new_node_for(init, cond, increment, then, for_token);

  *rest = token;
  return node;
}

Node *switch_stmt(Token **rest, Token *token) {
  Token *switch_token = token;
  if (!equal(token, "switch"))
    error_at(token->location, "expected switch");
  token = token->next;

  if (!equal(token, "("))
    error_at(token->location, "expected ( of switch statement");
  token = token->next;

  Node *cond = expr(&token, token);

  if (!equal(token, ")"))
    error_at(token->location, "expected ) of switch statement");
  token = token->next;

  if (!equal(token, "{"))
    error_at(token->location, "expected { of switch statement");
  token = token->next;

  Node stmt_head = {};
  Node *stmt_tail = &stmt_head;
  Node case_head = {};
  Node *case_tail = &case_head;
  int case_num = 1;
  bool have_default = false;
  while (!equal(token, "}")) {
    if (equal(token, "case")) {
      Token *case_token = token;
      case_tail = case_tail->next = expr(&token, token->next);

      if (!equal(token, ":"))
        error_at(token->location, "expected : of case label in switch statement");
      token = token->next;

      stmt_tail = stmt_tail->next = new_node_case(case_token, case_num++);
      continue;
    }
    if (equal(token, "default")) {
      if (!equal(token->next, ":"))
        error_at(token->next->location, "expected : of default label in switch statement");
      if (have_default)
        error_at(token->location, "duplicated default label in switch statement");

      have_default = true;
      stmt_tail = stmt_tail->next = new_node_default(token);
      token = token->next->next;
      continue;
    }

    stmt_tail->next = stmt_without_declaration(&token, token);
    while (stmt_tail->next)
      stmt_tail = stmt_tail->next;
  }
  *rest = token->next;
  return new_node_switch(cond, case_head.next, stmt_head.next, have_default, switch_token);
}

// return_stmt = return expr ";"
Node *return_stmt(Token **rest, Token *token) {
  Token *return_token = token;
  if (!equal(return_token, "return"))
    error_at(return_token->location, "expected return");
  token = return_token->next;

  Node *node = new_node_return(expr(&token, token), return_token);

  if (!equal(token, ";"))
    error_at(token->location, "expected ;");
  token = token->next;

  *rest = token;
  return node;
}

// expr_stmt  =  expr ";"
Node *expr_stmt(Token **rest, Token *token) {
  Token *expr_token = token;
  Node *node = new_node_expr_stmt(expr(&token, token), expr_token);

  if (!equal(token, ";"))
    error_at(token->location, "expected ;");
  token = token->next;

  *rest = token;
  return node;
}

// declare_lvar_stmt = type identifer type_suffix ("=" expr)? ";"
// type_suffix       = "[" num "]" type_suffix | ε
// declare node is skipped by codegen

Node *declare_lvar_stmt(Token **rest, Token *token, Type *ancestor) {
  // identifer
  if (!is_identifer_token(token)) {
    if (!equal(token,  ";") || ancestor->kind != TYPE_STRUCT)
      error_at(token->location, "expected identifer of new local variable");
    *rest = token-> next;
    return NULL;
  }

  char *name = token->location;
  int namelen = token->length;
  token = token->next;

  // ("[" num "]")*
  Type *type = type_suffix(&token, token, ancestor);

  if (find_var_in_vars(name, namelen, now_scope->vars) || find_in_typedefs(name, namelen, now_scope->tdfs))
    error("duplicate declarations '%.*s'", namelen, name);
  new_var(type, name, namelen);

  if (equal(token, ";")) {
    token = token->next;
    *rest = token;
    return NULL;
  }

  if (!equal(token, "="))
    error_at(token->location, "expected ; or =");
  token = token->next;

  Node *node = init_lvar_stmts(&token, token, now_scope->vars, NULL);

  if (!equal(token, ";"))
    error_at(token->location, "expected ;");
  token = token->next;

  *rest = token;
  return node;
}

Type *type_suffix(Token **rest, Token *token, Type *ancestor) {
  if (!equal(token, "["))
    return ancestor;
  token = token->next;

  int length = str_to_l(token->location, token->length);
  token = token->next;
  if (!equal(token,"]"))
    error_at(token->location, "expected ]");
  token = token->next;

  Type *parent = type_suffix(&token, token, ancestor);

  *rest = token;
  return new_type_array(parent, length);
}

void typedef_stmt(Token **rest, Token *token) {
  if (!equal(token, "typedef"))
     error_at(token->location, "expected 'typedef'");
  token = token->next;

  Type *type = read_type(&token, token, ALLOW_STATIC);

  if (!is_identifer_token(token))
    error_at(token->location, "expected identifer of typedef");
  new_typedef(type, token->location, token->length);
  token = token->next;

  if (!equal(token, ";"))
    error_at(token->location, "expected ; of the end of the typedef statement");
  token = token->next;

  *rest = token;
}
