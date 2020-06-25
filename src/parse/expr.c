#include "parse.h"

// expr       = assign
Node *expr(Token **rest, Token *token) {
  Node *node = assign(&token, token);
  *rest = token;
  return node;
}

// assign     = equality ("=" assign)?
Node *assign(Token **rest, Token *token) {
  Node *node = equality(&token, token);

  if(equal(token, "=")) {
    Token *op_token = token;
    node = new_node_assign(node, assign(&token, op_token->next), op_token);
  }

  *rest = token;
  return node;
}

// equality = relational ("==" relational | "!=" relational)*
Node *equality(Token **rest, Token *token) {
  Node *node = relational(&token, token);
  for(;;) {
    Token *op_token = token;

    if (equal(op_token, "==")){
      node = new_node_equal(node, relational(&token, op_token->next), op_token);
      continue;
    }
    if (equal(op_token, "!=")) {
      node = new_node_not_equal(node, relational(&token, op_token->next), op_token);
      continue;
    }

    *rest = token;
    return node;
  }
}

// relational = add ("<" add | "<=" add | ">" add | ">=" add)*
Node *relational(Token **rest, Token *token) {
  Node *node = add(&token, token);
  for(;;) {
    Token *op_token = token;
    if (equal(op_token, "<")){
      node = new_node_less_than(node, add(&token, op_token->next), op_token);
      continue;
    }
    if (equal(op_token, "<=")) {
      node = new_node_less_equal(node, add(&token, op_token->next), op_token);
      continue;
    }
    if (equal(op_token, ">")){
      node = new_node_less_than(add(&token, op_token->next), node, op_token);
      continue;
    }
    if (equal(op_token, ">=")) {
      node = new_node_less_equal(add(&token, op_token->next), node, op_token);
      continue;
    }

    *rest = token;
    return node;
  }
}

// add = mul ("+" mul | "-" mul)*
Node *add(Token **rest, Token *token) {
  Node *node = mul(&token, token);

  for(;;) {
    Token *op_token = token;
    if (equal(op_token, "+")){
      node = new_node_add(node, mul(&token, op_token->next), op_token);
      continue;
    }
    if (equal(op_token, "-")) {
      node = new_node_sub(node, mul(&token, op_token->next), op_token);
      continue;
    }

    *rest = token;
    return node;
  }
}

// mul = unary ("*" unary | "/" unary)*
Node *mul(Token **rest, Token *token) {
  Node *node = unary(&token, token);

  for(;;) {
    Token *op_token = token;
    if (equal(op_token, "*")) {
      node = new_node_mul(node, unary(&token, op_token->next), op_token);
      continue;
    }
    if (equal(op_token, "/")) {
      node = new_node_div(node, unary(&token, op_token->next), op_token);
      continue;
    }

    *rest = token;
    return node;
  }
}

static Node *postfix(Token **rest, Token *token, Node *primary_node);
// unary = "+" primary
//       | "-" primary
//       | "sizeof" unary
//       | "*" unary
//       | "&" unary
//       | primary postfix
Node *unary(Token **rest, Token *token) {
  Node *node;
  Token *op_token = token;

  if (equal(op_token,"+")) {
    node = primary(&token, op_token->next);
  } else if (equal(op_token,"-")) {
    node = new_node_sub(new_node_num(0, op_token), primary(&token, op_token->next), op_token);
  } else if (equal(op_token, "*")) {
    node = new_node_deref(unary(&token, op_token->next), op_token);
  } else if (equal(op_token, "&")) {
    node = new_node_addr(unary(&token, op_token->next), op_token);
  } else if (equal(op_token, "sizeof")) {
    node = sizeofunary(&token, token);
  } else {
    node = primary(&token, token);
    node = postfix(&token, token ,node);
  }

  *rest = token;
  return node;
}

// sizeofunary = "sizeof" ( type_with_pars | unary )
// type_with_pars = "(" type_with_pars ")" | type
Node *sizeofunary(Token **rest, Token *token) {
  Token *op_token = token;
  if (!equal(op_token, "sizeof"))
    error_at(op_token->location, "expected sizeof");

  token = op_token->next;

  int pars = 0;
  while (equal(token, "(")) {
    token = token->next;
    pars++;
  }

  Type *type = read_type(&token, token, DENY_STATIC);

  if (type) {
    for (int i=0; i<pars; i++) {
      if (!equal(token, ")"))
        error_at(token->location, "expected )");
      token = token->next;
    }
  } else {
    Node *measuring_node = unary(&token, op_token->next);
    type = measuring_node->type;
  }

  *rest = token;
  return new_node_num(type->size, op_token);
}

static Node *postfix(Token **rest, Token *token, Node *primary_node) {
  Node *node = primary_node;

  // ( "[" expr "]" | "." identifer | "->" identifer)*
  for (;;) {
    if (equal(token, "[")) {
      Token *bracket_token = token;
      token = token->next;

      node = new_node_deref(new_node_add(node, expr(&token,token), bracket_token), bracket_token);

      if (!equal(token, "]")) {
        error_at(token->location, "expected ]");
      }
      token = token->next;
      continue;
    }
    if (equal(token, ".")) {
      token = token->next;

      if (!is_identifer_token(token))
        error_at(token->location, "expected identifer of struct member");
      node = new_node_member(node, token->location, token->length, token);
      token = token->next;
      continue;
    }
    if (equal(token, "->")) {
      Token *deref_token = token;
      token = token->next;
      if (!is_identifer_token(token))
        error_at(token->location, "expected identifer of struct member");
      node = new_node_member( new_node_deref(node, deref_token), token->location, token->length, token);
      token = token->next;
      continue;
    }
    *rest = token;
    return node;
  }
}

// primary    = num
//            | ' ... '
//            | primary_identifer
//            | string
//            | "(" expr ")"
//            | "(" "{" stmt+ "}" ")"
Node *primary(Token **rest, Token *token) {
  Node *node;

  // num
  if (is_number_token(token)) {
    node = new_node_num(strtol(token->location, NULL, 10), token);
    *rest = token->next;
    return node;
  }

  // ' ... '
  if (is_char_token(token)) {
    char c = (token->length == 3) ? *(token->location+1) : get_escape_char(*(token->location+2));
    node = new_node_num(c, token);
    node->type = new_type_char();
    *rest = token->next;
    return node;
  }

  // string
  if (is_string_token(token)) {
    node = new_node_string(token);
    *rest = token->next;
    return node;
  }

  // "(" expr ")"  | "(" "{" stmt+ "}" ")"
  if (equal(token,"(")) {
    if (equal(token->next, "{")) {
      // stmt_expr
      node = stmt_expr(&token, token);
      *rest = token;
      return node;
    }

    node = expr(&token, token->next);

    if (!equal(token,")"))
      error_at(token->location, "expected )");

    *rest = token->next;
    return node;
  }

  // primary_identifer
  if (is_identifer_token(token)) {
    node = primary_identifer(&token, token);
    *rest = token;
    return node;
  }

  error_at(token->location, "expected primary");
}

// primary_identifer = ident ( "(" ")" )?
Node *primary_identifer(Token **rest, Token *token) {
  // identifer
  Token *ident_token = token;
  if (!is_identifer_token(ident_token))
    error_at(ident_token->location, "expected identifer of variable or function name");

  char *name = ident_token->location;
  int namelen = ident_token->length;
  token = ident_token->next;

  if (!equal(token, "(")) {
    *rest = token;
    return new_node_var(name,namelen, ident_token);
  }

  // "(" ... ")"
  token = token->next;

  Node args_head = {};
  Node *args_tail = &args_head;

  while(!equal(token, ")")) {
    args_tail->next = expr(&token, token);
    args_tail = args_tail->next;

    if (!equal(token, ","))
      break;
    token = token->next;
  }

  if (!equal(token, ")"))
    error_at(token->location, "expected )");

  *rest = token->next;
  return new_node_func_call(name, namelen, args_head.next, ident_token);
}

Node *stmt_expr(Token **rest, Token *token) {
  if (!equal(token, "(") || !equal(token->next, "{"))
    error_at(token->next->location, "expected statement expression");
  token = token->next;

  scope_in();
  Node *node = block_stmt(&token, token);
  scope_out();
  node->kind = ND_STMT_EXPR;

  Node *tail = node->body;
  while (tail && tail->next)
    tail = tail->next;
  node->type = (tail && tail->kind == ND_EXPR_STMT) ?  tail->left->type : NULL;

  if (!equal(token, ")"))
    error_at(token->location, "expected )");
  token = token->next;

  *rest = token;
  return node;
}
