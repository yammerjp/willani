#include "parse.h"

static Node *new_node_op2(NodeKind kind, Node *left, Node *right, Token *token) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = kind;
  node->left = left;
  node->right = right;
  node->type = type_conversion(node->left->type, node->right->type);
  node->token = token;
  return node;
}

static bool ptr_or_arr(Node *node) {
  return node->type->kind == TYPE_PTR || node->type->kind == TYPE_ARRAY;
}

  // single operator
//  ND_EXPR_VAR,          // Variable
Node *new_node_var(char *name, int length, Token *token) {
  Var *var = find_var(name, length);
  if (!var)
    error_at(name, "use undeclared identifer");
  if (var->is_typedef)
    error_at(token->location, "expected a variable but typedef");
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_EXPR_VAR;
  node->var = var;
  node->type = var->type;
  node->token = token;
  return node;
}

//  ND_EXPR_NUM,          // Integer
Node *new_node_num(long value, Token *token) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_EXPR_NUM;
  node->value = value;
  node->type = new_type_int();
  node->token = token;
  return node;
}

//  ND_EXPR_STRING,       // " ... "
Node *new_node_string(Token *token) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_EXPR_STRING;
  node->string = new_string(token->location+1, token->length-2);
  node->type = new_type_array(new_type_char(), node->string->length);
  node->token = token;
  return node;
}


//  ND_EXPR_FUNC_CALL,    // Function call
Node *new_node_func_call(char *name, int len, Node *args, Token *token) {
  Function *func = find_function(name, len);
  if (!func)
    error_at(token->location, "called undefined function");

  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_EXPR_FUNC_CALL;
  node->func_name = name;
  node->func_args = args;
  node->func_namelen = len;
  node->type = func->type;
  node->token = token;

  return node;
}

//  ND_EXPR_ADDR,         // & ...
Node *new_node_addr(Node *unary_node, Token *token) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_EXPR_ADDR;
  node->left = unary_node;
  node->type = new_type_pointer(unary_node->type);
  node->token = token;
  return node;
}

//  ND_EXPR_DEREF,        // * ...
Node *new_node_deref(Node *unary_node, Token *token) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_EXPR_DEREF;
  node->left = unary_node;
  node->type = unary_node->type->base;
  node->token = token;
  return node;
}

//  ND_EXPR_NOT,          // ! ...
Node *new_node_not(Node *left, Token *token) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_EXPR_NOT;
  node->left = left;
  node->type = new_type_int();
  node->token = token;
  return node;
}

//  ND_EXPR_BIT_NOT,      // ~ ...
Node *new_node_bit_not(Node *left, Token *token) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_EXPR_BIT_NOT;
  node->left = left;
  node->type = left->type;
  node->token = token;
  return node;
}


//  ND_EXPR_ASSIGN,       // =
Node *new_node_assign(Node *left, Node *right, Token *token) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_EXPR_ASSIGN;
  node->left = left;
  node->right = right;
  node->type = left->type;
  node->token = token;
  return node;
}

//  ND_EXPR_ADD,          // +
Node *new_node_add(Node *left, Node *right, Token *token) {
  Node *num;
  if (ptr_or_arr(left)) {
    num = new_node_num(left->type->base->size, left->token);
    right = new_node_mul(right, num, right->token);
  } else if (ptr_or_arr(right)) {
    num = new_node_num(right->type->base->size, right->token);
    left = new_node_mul(left, num, left->token);
  }
  return new_node_op2(ND_EXPR_ADD, left, right, token);
}

//  ND_EXPR_SUB,          // -
Node *new_node_sub(Node *left, Node *right, Token *token) {
  Node *sub;
  Node *num;
  if (ptr_or_arr(left) && ptr_or_arr(right)) {
    sub =  new_node_op2(ND_EXPR_SUB, left, right, token);
    num = new_node_num(left->type->base->size, token);
    return new_node_div(sub, num, token);
  }
  if (ptr_or_arr(left)) {
    num = new_node_num( left->type->base->size, left->token );
    right = new_node_mul(right, num, right->token);
  } else if (ptr_or_arr(right)) {
    num = new_node_num( right->type->base->size, right->token );
    left = new_node_mul(left, num, left->token);
  }
  return new_node_op2(ND_EXPR_SUB, left, right, token);
}

//  ND_EXPR_MUL,          // *
Node *new_node_mul(Node *left, Node *right, Token *token) {
  return new_node_op2(ND_EXPR_MUL, left, right, token);
}

//  ND_EXPR_DIV,          // /
Node *new_node_div(Node *left, Node *right, Token *token) {
  return new_node_op2(ND_EXPR_DIV, left, right, token);
}

//  ND_EXPR_MOD,          // %
Node *new_node_mod(Node *left, Node *right, Token *token) {
  return new_node_op2(ND_EXPR_MOD, left, right, token);
}

//  ND_EXPR_EQ,           // ==
Node *new_node_equal(Node *left, Node *right, Token *token) {
  Node *node = new_node_op2(ND_EXPR_EQ, left, right, token);
  node->type = new_type_int();
  return node;
}

//  ND_EXPR_NE,           // !=
Node *new_node_not_equal(Node *left, Node *right, Token *token) {
  Node *node = new_node_op2(ND_EXPR_NE, left, right, token);
  node->type = new_type_int();
  return node;
}

//  ND_EXPR_LESS_THAN,    // <
Node *new_node_less_than(Node *left, Node *right, Token *token) {
  Node *node = new_node_op2(ND_EXPR_LESS_THAN, left, right, token);
  node->type = new_type_int();
  return node;
}

//  ND_EXPR_LESS_EQ,      // <=
Node *new_node_less_equal(Node *left, Node *right, Token *token) {
  Node *node = new_node_op2(ND_EXPR_LESS_EQ, left, right, token);
  node->type = new_type_int();
  return node;
}

//  ND_EXPR_MEMBER,       // . struct member access
Node *new_node_member(Node *parent, char *name, int namelen,  Token *token) {
  Member *member = find_member(parent->type, name, namelen);
  if (!member)
    error_at(token->location, "refered undefined member of struct");
  
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_EXPR_MEMBER;
  node->left = parent;
  node->type = member->type;
  node->member = member;
  node->token = token;
  return node;
}

//  ND_EXPR_COMMA,        // ... , ...
Node *new_node_comma(Node *left, Node *right, Token *token) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_EXPR_COMMA;
  node->left = left;
  node->right = right;
  node->type = right->type;
  node->token = token;
  return node;
}


//  ND_EXPR_WITH_STMTS,   // ({ ...; ... }) GNU statement expression extention
//  created in src/parse/expr.c

//  ND_STMT_BLOCK,        // { ... }
Node *new_node_block(Node *body, Token *token) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_STMT_BLOCK;
  node->body = body;
  node->token = token;
  return node;
}

//  ND_STMT_IF,           // if
Node *new_node_if(Node *cond, Node *then, Node *els, Token *token) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_STMT_IF;
  node->cond = cond;
  node->then = then;
  node->els = els;
  node->token = token;
  return node;
}

//  ND_STMT_SWITCH,       // switch { ... }
Node *new_node_switch(Node *cond, Node *cases, Node *body, Token *token) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_STMT_SWITCH;
  node->cond = cond;
  node->cases = cases;
  node->body = body;
  node->token = token;
}

//  ND_STMT_WHILE,        // while
Node *new_node_while(Node *cond, Node *then, Token *token) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_STMT_WHILE;
  node->cond = cond;
  node->then = then;
  node->token = token;
  return node;
}

//  ND_STMT_FOR,          // for
Node *new_node_for(Node *init, Node *cond, Node* increment, Node *then, Token *token) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_STMT_FOR;
  node->init = init;
  node->increment = increment;
  node->cond = cond;
  node->then = then;
  node->token = token;
  return node;
}

//  ND_STMT_RETURN,       // return
Node *new_node_return(Node *left, Token *token) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_STMT_RETURN;
  node->left = left;
  node->token = token;
  return node;
}

//  ND_STMT_WITH_EXPR,    // ... ;
Node *new_node_expr_stmt(Node *expr_node, Token *token) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_STMT_WITH_EXPR;
  node->left = expr_node;
  node->token = token;
  return node;
}

//  ND_STMT_CONTINUE,     // continue;
Node *new_node_continue(Token *token) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_STMT_CONTINUE;
  node->token = token;
  return node;
}

//  ND_STMT_BREAK,        // break;
Node *new_node_break(Token *token) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_STMT_BREAK;
  node->token = token;
  return node;
}

//  ND_LABEL_CASE,        // case expr: in switch statement
Node *new_node_case(Token *token, int case_num) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_LABEL_CASE;
  node->token = token;
  node->case_num = case_num;
  return node;
}

//  ND_LABEL_DEFAULT,     // default: in switch statement
Node *new_node_default(Token *token) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_LABEL_DEFAULT;
  node->token = token;
  return node;
}
