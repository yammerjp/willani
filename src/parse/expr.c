#include "parse.h"

// expr       = assign
Node *expr(Token **rest, Token *token, Var **lvarsp) {
  Node *node = assign(&token, token, lvarsp);
  *rest = token;
  return node;
}

// assign     = equality ("=" assign)?
Node *assign(Token **rest, Token *token, Var **lvarsp) {
  Node *node = equality(&token, token, lvarsp);

  if(equal(token, "=")) {
    token = token->next;
    node = new_node_assign(node, assign(&token, token, lvarsp));
  }

  *rest = token;
  return node;
}

// equality = relational ("==" relational | "!=" relational)*
Node *equality(Token **rest, Token *token, Var **lvarsp) {
  Node *node = relational(&token, token, lvarsp);
  for(;;) {
    if (equal(token, "==")){
      token = token->next;
      node = new_node_equal(node, relational(&token, token, lvarsp));
    } else if (equal(token, "!=")) {
      token = token->next;
      node = new_node_not_equal(node, relational(&token, token, lvarsp));
    } else {
      *rest = token;
      return node;
    }
  }
}

// relational = add ("<" add | "<=" add | ">" add | ">=" add)*
Node *relational(Token **rest, Token *token, Var ** lvarsp) {
  Node *node = add(&token, token, lvarsp);
  for(;;) {
    if (equal(token, "<")){
      token = token->next;
      node = new_node_less_than(node, add(&token, token, lvarsp));
      continue;
    }
    if (equal(token, "<=")) {
      token = token->next;
      node = new_node_less_equal(node, add(&token, token, lvarsp));
      continue;
    }
    if (equal(token, ">")){
      token = token->next;
      node = new_node_less_than(add(&token, token, lvarsp), node);
      continue;
    }
    if (equal(token, ">=")) {
      token = token->next;
      node = new_node_less_equal(add(&token, token, lvarsp), node);
      continue;
    }
    *rest = token;
    return node;
  }
}

// add = mul ("+" mul | "-" mul)*
Node *add(Token **rest, Token *token, Var **lvarsp) {
  Node *node = mul(&token, token, lvarsp);

  for(;;) {
    if (equal(token, "+")){
      token = token->next;
      node = new_node_add(node, mul(&token, token, lvarsp));
    } else if (equal(token, "-")) {
      token = token->next;
      node = new_node_sub(node, mul(&token, token, lvarsp));
    } else {
      *rest = token;
      return node;
    }
  }
}

// mul = unary ("*" unary | "/" unary)*
Node *mul(Token **rest, Token *token, Var **lvarsp) {
  Node *node = unary(&token, token, lvarsp);

  for(;;) {
    if (equal(token, "*")) {
      token = token->next;
      node = new_node_mul(node, unary(&token, token, lvarsp));
    } else if (equal(token, "/")) {
      token = token->next;
      node = new_node_div(node, unary(&token, token, lvarsp));
    } else {
      *rest = token;
      return node;
    }
  }
}

// unary = ("+" | "-")? primary
//       | "sizeof" unary
//       | ( "*" | "&" ) unary
//       | primary ("[" expr "]")*
Node *unary(Token **rest, Token *token, Var **lvarsp) {
  Node *node;
  if (equal(token,"+")) {
    token = token->next;
    node = primary(&token, token, lvarsp);
  } else if (equal(token,"-")) {
    token = token->next;
    node = new_node_sub(new_node_num(0), primary(&token, token, lvarsp));
  } else if (equal(token, "*")) {
    token = token->next;
    node = new_node_deref(unary(&token, token, lvarsp));
  } else if (equal(token, "&")) {
    token = token->next;
    node = new_node_addr(unary(&token, token, lvarsp));
  } else if (equal(token, "sizeof")) {
    node = sizeofunary(&token, token, lvarsp);
  } else {
    node = primary(&token, token, lvarsp);

    // ("[" expr "]")*
    while(equal(token, "[")) {
      token = token->next;

      Node *expr_node = expr(&token,token, lvarsp);
      node = new_node_deref(new_node_add(node, expr_node));

      if (!equal(token, "]")) {
        error_at(token, "expected ]");
      }
      token = token->next;
    }
  }
  *rest = token;
  return node;
}

// sizeofunary = "sizeof" ( type_with_pars | unary )
// type_with_pars = "(" type_with_pars ")" | type
Node *sizeofunary(Token **rest, Token *token, Var **lvarsp) {
  if (!equal(token, "sizeof")) {
    error_at(token, "expected sizeof");
  }
  token = token->next;

  Type *type = read_type_tokens_with_pars(&token, token);

  if (!type) {
    Node *measuring_node = unary(&token, token, lvarsp);
    type = measuring_node->type;
  }
  int size = type_size(type);

  *rest = token;
  return new_node_num(size);
}

// primary    = num | primary_identifer | "(" expr ")"
Node *primary(Token **rest, Token *token, Var **lvarsp) {
  Node *node;

  // num
  if (is_number_token(token)) {
    node = new_node_num(strtol(token->location, NULL, 10));
    token = token->next;
    *rest = token;
    return node;
  }

  // primary_identifer
  if (is_identifer_token(token)) {
    node = primary_identifer(&token, token, lvarsp);
    *rest = token;
    return node;
  }

  // "(" expr ")"
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

// primary_identifer = ident ( "(" ")" )?
Node *primary_identifer(Token **rest, Token *token, Var **lvarsp) {
  Node *node;

  // identifer
  if (!is_identifer_token(token)) {
    error_at(token, "expected identifer");
  }
  char *name = token->location;
  int length = token->length;
  token = token->next;

  if (!equal(token, "(")) {
    node = new_node_var(name,length, *lvarsp);
    *rest = token;
    return node;
  }

  // "(" ... ")"
  token = token->next;

  Node args_head = {};
  Node *args_tail = &args_head;

  for(int i=0;i<6;i++) {
    if (equal(token, ")")) {
      break;
    }
    args_tail->next = expr(&token, token, lvarsp);
    args_tail = args_tail->next;
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

  *rest = token;
  return node;
}
