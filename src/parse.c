#include "willani.h"

static Function *function(Token **rest, Token *token);
static Node *stmt(Token **rest, Token *token, LVar **lvarsp);
static Node *if_stmt(Token **rest, Token *token, LVar **lvarsp);
static Node *while_stmt(Token **rest, Token *token, LVar **lvarsp);
static Node *for_stmt(Token **rest, Token *token, LVar **lvarsp);
static Node *block_stmt(Token **rest, Token *token, LVar **lvarsp);
static Node *expr_stmt(Token **rest, Token *token, LVar **lvarsp);
static Node *return_stmt(Token **rest, Token *token, LVar **lvarsp);
static Node *declare_lvar_stmt(Token **rest, Token *token, LVar **lvarsp, Type *type);
static Node *expr(Token **rest, Token *token, LVar **lvarsp);
static Node *assign(Token **rest, Token *token, LVar **lvarsp);
static Node *equality(Token **rest, Token *token, LVar **lvarsp);
static Node *relational(Token **rest, Token *token, LVar **lvarsp);
static Node *add(Token **rest, Token *token, LVar **lvarsp);
static Node *mul(Token **rest, Token *token, LVar **lvarsp);
static Node *unary(Token **rest, Token *token, LVar **lvarsp);
static Node *sizeofunary(Token **rest, Token *token, LVar **lvarsp);
static Node *primary(Token **rest, Token *token, LVar **lvarsp);

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

// function = ( type ident "(" ( ( type ident ( "," type ident ) * ) ?  ")" block_stmt ) *

Function *function(Token **rest, Token *token) {
  LVar *lvars = NULL;

  // type
  Type *return_type = read_type_tokens(&token, token);

  // function name
  if (!is_identifer_token(token)) {
    error_at(token, "expected identifer");
  }

  char *name = token->location;
  int length = token->length;
  token = token->next;

  // arguments
  if (!equal(token, "(")) {
    error_at(token, "expected (");
  }
  token = token->next;

  int argc = 0;

  while (!equal(token, ")")) {
    Type *arg_type = read_type_tokens(&token, token);

    argc++;

    if(!is_identifer_token(token)) {
      error_at(token, "expected identifer");
    }
    new_lvar(arg_type, token->location, token->length, &lvars);
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

  LVar *args = lvars;

  // block statement
  Node *node = block_stmt(&token, token, &lvars);
  add_type(node);

  // create Function struct
  Function *func = calloc(1, sizeof(Function));
  func->node = node;
  func->lvar = lvars;
  func->name = name;
  func->args = args;
  func->argc = argc;
  func->namelen = length;
  func->type = return_type;

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
//            | declare_lvar_stmt
static Node *stmt(Token **rest, Token *token, LVar **lvarsp) {
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

// declare_lvar_stmt = type identifer ";"
// declare node is skipped by codegen
static Node *declare_lvar_stmt(Token **rest, Token *token, LVar **lvarsp, Type *type) {

  if(!is_identifer_token(token)) {
    error_at(token, "expected identifer");
  }

  Node *node = new_node_declare_lvar(type, token->location, token->length, lvarsp);
  token = token->next;

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
//       | "sizeof" unary
//       | ( "*" | "&" ) unary
static Node *unary(Token **rest, Token *token, LVar **lvarsp) {
  Node *node;
  if (equal(token,"+")) {
    token = token->next;
    node = primary(&token, token, lvarsp);
  } else if (equal(token,"-")) {
    token = token->next;
    node = new_node_op2(ND_SUB, new_node_num(0), primary(&token, token, lvarsp));
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
  }
  *rest = token;
  return node;
}

// sizeofunary = "sizeof" ( type_with_pars | unary )
// type_with_pars = "(" type_with_pars ")" | type
static Node *sizeofunary(Token **rest, Token *token, LVar **lvarsp) {
  if (!equal(token, "sizeof")) {
    error_at(token, "expected sizeof");
  }
  token = token->next;

  Type *type = read_type_tokens_with_pars(&token, token);

  if (!type) {
    Node *measuring_node = unary(&token, token, lvarsp);
    add_type(measuring_node);
    type = measuring_node->type;
  }
  int size = type_size(type);

  *rest = token;
  return new_node_num(size);
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
      node = new_node_lvar(token->location, token->length, *lvarsp);
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
