#include "parse.h"

// expr       = assign ("," assign)*
Node *expr(Token **rest, Token *token) {
  parse_log("expr()");
  Node *node = assign(&token, token);
  while (equal(token, ",")) {
    Token *comma_token = token;
    node = new_node_expr_stmt(node, node->token);
    node = new_node_comma(node, assign(&token, token->next), comma_token);
  }
  *rest = token;
  return node;
}

// assign     = ternary (("=" | "+=" | "-=" | "*=" | "/=" | "%=") assign )?
Node *assign(Token **rest, Token *token) {
  parse_log("assign()");
  Node *node = ternary(&token, token);
  Node *left = node;
  Token *op_token = token;

  if (!(node->type))
    error_at(node->token, "the node's type is NULL");
  if (node->type->is_const) {
    *rest = token;
    return node;
  }

  if(equal(token, "="))
    node = new_node_assign(left, assign(&token, op_token->next), op_token);
  else if (equal(token, "+="))
    node = new_node_assign_add(left, assign(&token, op_token->next), op_token);
  else if (equal(token, "-="))
    node = new_node_assign_sub(left, assign(&token, op_token->next), op_token);
  else if (equal(token, "*="))
    node = new_node_assign_mul(left, assign(&token, op_token->next), op_token);
  else if (equal(token, "/="))
    node = new_node_assign_div(left, assign(&token, op_token->next), op_token);
  else if (equal(token, "%="))
    node = new_node_assign_mod(left, assign(&token, op_token->next), op_token);
  else if (equal(token, "<<="))
    node = new_node_assign_shift_left(left, assign(&token, op_token->next), op_token);
  else if (equal(token, ">>="))
    node = new_node_assign_shift_right(left, assign(&token, op_token->next), op_token);

  *rest = token;
  return node;
}

// ternary = log_or ("?" expr ":" ternary)?
Node *ternary(Token **rest, Token *token) {
  parse_log("ternary()");
  Node *node = log_or(&token, token);

  if (equal(token, "?")) {
    Token *op_token = token;
    Node *left = expr(&token, token->next);
    if (!equal(token, ":"))
      error_at(token, "expected ':' of ternary operator");
    Node *right = ternary(&token, token->next);
    node = new_node_ternary(node, left, right, op_token);
  }
  *rest = token;
  return node;
}

// log_or  = log_and ("||" log_or)?
Node *log_or(Token **rest, Token *token) {
  parse_log("log_or()");
  Node *node = log_and(&token, token);

  if (equal(token, "||")) {
    Token *op_token = token;
    node = new_node_logical_or(node, log_or(&token, token->next), op_token);
  }
  *rest = token;
  return node;
}

// log_and = bit_or ("&&" log_and) ?
Node *log_and(Token **rest, Token *token) {
  parse_log("log_and()");
  Node *node = bit_or(&token, token);
  if (equal(token, "&&")) {
    Token *op_token = token;
    node = new_node_logical_and(node, log_and(&token, token->next), op_token);
  }
  *rest = token;
  return node;
}

// bit_or   = bit_xor ("|" bit_or)?
Node *bit_or(Token **rest, Token *token) {
  parse_log("log_or()");
  Node *node = bit_xor(&token, token);
  if (equal(token, "|")) {
    Token *op_token = token;
    node = new_node_bit_or(node, bit_or(&token, token->next), op_token);
  }
  *rest = token;
  return node;
}


// bit_xor   = bit_and ("|" bit_xor)?
Node *bit_xor(Token **rest, Token *token) {
  parse_log("log_xor()");
  Node *node = bit_and(&token, token);
  if (equal(token, "^")) {
    Token *op_token = token;
    node = new_node_bit_xor(node, bit_xor(&token, token->next), op_token);
  }
  *rest = token;
  return node;
}

// bit_and  = equality ("&" bit_and)?
Node *bit_and(Token **rest, Token *token) {
  parse_log("bit_and()");
  Node *node = equality(&token, token);
  if (equal(token, "&")) {
    Token *op_token = token;
    node = new_node_bit_and(node, equality(&token, token->next), op_token);
  }
  *rest = token;
  return node;
}

// equality = shift ("==" shift | "!=" shift)*
Node *equality(Token **rest, Token *token) {
  parse_log("equality()");
  Node *node = shift(&token, token);
  for(;;) {
    Token *op_token = token;

    if (equal(op_token, "==")){
      node = new_node_equal(node, shift(&token, op_token->next), op_token);
      continue;
    }
    if (equal(op_token, "!=")) {
      node = new_node_not_equal(node, shift(&token, op_token->next), op_token);
      continue;
    }

    *rest = token;
    return node;
  }
}

// shift = relational (">>" relational | "<<" relational)*
Node *shift(Token **rest, Token *token) {
  parse_log("shift()");
  Node *node = relational(&token, token);
  for (;;) {
    Token *op_token = token;

    if (equal(op_token, "<<")) {
      node = new_node_shift_left(node, relational(&token, op_token->next), op_token);
      continue;
    }
    if (equal(op_token, ">>")) {
      node = new_node_shift_right(node, relational(&token, op_token->next), op_token);
      continue;
    }
    *rest = token;
    return node;
  }
}

// relational = add ("<" add | "<=" add | ">" add | ">=" add)*
Node *relational(Token **rest, Token *token) {
  parse_log("relational()");
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
  parse_log("add()");
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

// mul = unary (("*" | "/" | "%") unary )*
Node *mul(Token **rest, Token *token) {
  parse_log("mul()");
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
    if (equal(op_token, "%")) {
      node = new_node_mod(node, unary(&token, op_token->next), op_token);
      continue;
    }

    *rest = token;
    return node;
  }
}

static Node *postfix(Token **rest, Token *token, Node *primary_node);
// unary = "+" unary
//       | "-" unary
//       | "!" unary
//       | "~" unary
//       | "++" unary
//       | "--" unary
//       | "sizeof" unary
//       | "*" unary
//       | "&" unary
//       | primary postfix
Node *unary(Token **rest, Token *token) {
  parse_log("unary()");
  Node *node;
  Token *op_token = token;

  if (equal(op_token,"+")) {
    node = unary(&token, op_token->next);
  } else if (equal(op_token,"-")) {
    node = new_node_sub(new_node_num(0, op_token), unary(&token, op_token->next), op_token);
  } else if (equal(op_token,"!")) {
    node = new_node_not(unary(&token, op_token->next), op_token);
  } else if (equal(op_token,"~")) {
    node = new_node_bit_not(unary(&token, op_token->next), op_token);
  } else if (equal(op_token, "*")) {
    node = new_node_deref(unary(&token, op_token->next), op_token);
  } else if (equal(op_token, "&")) {
    node = new_node_addr(unary(&token, op_token->next), op_token);
  } else if (equal(op_token, "sizeof")) {
    node = sizeofunary(&token, token->next, false);
  } else if (equal(op_token, "++")) {
    node = new_node_pre_increment(unary(&token, op_token->next), op_token);
  } else if (equal(op_token, "--")) {
    node = new_node_pre_decrement(unary(&token, op_token->next), op_token);
  } else if (equal(op_token, "(") && is_type_tokens(op_token->next, DENY_STATIC, DENY_EXTERN, DENY_CONST)) {
    Type *type = read_type(&token, op_token->next, DENY_STATIC, DENY_EXTERN, DENY_CONST);
    type = type_ptr_suffix(&token, token, type);
    if (!equal(token, ")"))
      error_at(token, "expected ) of type cast");
    node = new_node_cast(unary(&token, token->next), type, op_token->next);
  } else {
    node = primary(&token, token);
    node = postfix(&token, token ,node);
  }

  *rest = token;
  return node;
}

// unary of sizeof          = "sizeof" sizeofunary
// sizeofunary              = "(" sizeofunary_inner_parens ")" | type | unary
// sizeofunary_inner_parens = "(" sizeofunary_inner_parens ")" | type | expr
Node *sizeofunary(Token **rest, Token *token, bool inner_parens) {
  parse_log("sizeofunary()");
  Token *begin = token;
  if (equal(token, "(")) {
    Node *node = sizeofunary(&token, token->next, true);
    if (equal(token, ")")) {
      *rest = token->next;
      return node;
    }
    token = begin;
  }

  Type *type;
  if (is_type_tokens(token, DENY_STATIC, DENY_EXTERN, DENY_CONST)) {
    type = read_type(&token, token, DENY_STATIC, DENY_EXTERN, DENY_CONST);
    type = type_ptr_suffix(&token, token, type);
    if (!type)
      error_at(token, "expected type tokens");
  } else {
    Node *node = inner_parens ? expr(&token, token) : unary(&token, token);
    type = node->type;
  }
  *rest = token;
  return new_node_num(type->size, token);
}

static Node *postfix(Token **rest, Token *token, Node *primary_node) {
  Node *node = primary_node;

  // ( "[" expr "]" | "." identifer | "->" identifer | "++" | "--")*
  for (;;) {
    Token *op_token = token;
    if (equal(token, "[")) {
      token = token->next;

      node = new_node_deref(new_node_add(node, expr(&token,token), op_token), op_token);

      if (!equal(token, "]")) {
        error_at(token, "expected ]");
      }
      token = token->next;
      continue;
    }
    if (equal(token, ".")) {
      token = token->next;

      if (!is_identifer_token(token))
        error_at(token, "expected identifer of struct member");
      node = new_node_member(node, token->location, token->length, token);
      token = token->next;
      continue;
    }
    if (equal(token, "->")) {
      token = token->next;
      if (!is_identifer_token(token))
        error_at(token, "expected identifer of struct member");
      node = new_node_member( new_node_deref(node, op_token), token->location, token->length, token);
      token = token->next;
      continue;
    }
    if (equal(token, "++")) {
      node = new_node_post_increment(node, op_token);
      token = token->next;
      continue;
    }
    if (equal(token, "--")) {
      node = new_node_post_decrement(node, op_token);
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
  parse_log("primary()");
  Node *node;

  // num
  if (is_number_token(token)) {
    int num = str_to_l(token->location, token->length);
    node = new_node_num(num, token);
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
      error_at(token, "expected )");

    *rest = token->next;
    return node;
  }

  // primary_identifer
  if (is_identifer_token(token)) {
    node = primary_identifer(&token, token);
    *rest = token;
    return node;
  }

  error_at(token, "expected primary");
}

// primary_identifer = ident ( "(" ")" )?
Node *primary_identifer(Token **rest, Token *token) {
  parse_log("primary_identifer()");
  // identifer
  Token *ident_token = token;
  if (!is_identifer_token(ident_token))
    error_at(ident_token, "expected identifer of variable or function name");

  char *name = ident_token->location;
  int namelen = ident_token->length;
  token = ident_token->next;

  if (!equal(token, "(")) {
    *rest = token;
    Enum *enm = find_enum(name, namelen);
    if (enm)
      return new_node_num(enm->num, token);
    return new_node_var(name,namelen, ident_token);
  }

  // "(" ... ")"
  token = token->next;

  Node args_head = {};
  Node *args_tail = &args_head;

  while(!equal(token, ")")) {
    args_tail->next = assign(&token, token);
    args_tail = args_tail->next;

    if (!equal(token, ","))
      break;
    token = token->next;
  }

  if (!equal(token, ")"))
    error_at(token, "expected ) of calling function");

  *rest = token->next;
  return new_node_func_call(name, namelen, args_head.next, ident_token);
}

Node *stmt_expr(Token **rest, Token *token) {
  parse_log("stmt_expr()");
  if (!equal(token, "(") || !equal(token->next, "{"))
    error_at(token->next, "expected statement expression");
  token = token->next;

  scope_in();
  Node *node = block_stmt(&token, token);
  scope_out();
  node->kind = ND_EXPR_WITH_STMTS;

  Node *tail = node->body;
  while (tail && tail->next)
    tail = tail->next;
  node->type = (tail && tail->kind == ND_STMT_WITH_EXPR) ?  tail->left->type : NULL;

  if (!equal(token, ")"))
    error_at(token, "expected )");
  token = token->next;

  *rest = token;
  return node;
}
