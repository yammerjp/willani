#include "parse.h"

static Node *new_node_expr(NodeKind kind, Type *type, Token *token) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = kind;
  node->type = type;
  node->token = token;
  return node;
}

static Node *new_node_stmt(NodeKind kind, Token *token) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = kind;
  node->token = token;
  return node;
}


static Node *new_node_op2(NodeKind kind, Node *left, Node *right, Token *token) {
  Node *node = new_node_expr(kind, type_conversion(left->type, right->type), token);
  node->left = left;
  node->right = right;
  return node;
}

  // single operator
//  ND_EXPR_VAR,          // Variable
Node *new_node_var(char *name, int length, Token *token) {
  Var *var = find_var(name, length);
  if (!var)
    error_at(token, "use undeclared identifer");
  return new_node_var_specified(var, token);
}
Node *new_node_var_specified(Var *var, Token *token) {
  Node *node = new_node_expr(ND_EXPR_VAR, var->type, token);
  node->var = var;
  return node;
}

//  ND_EXPR_NUM,          // Integer
Node *new_node_num(long value, Token *token) {
  Node *node = new_node_expr(ND_EXPR_NUM, new_type_int(), token);
  node->value = value;
  return node;
}

//  ND_EXPR_STRING,       // " ... "
Node *new_node_string(Token *token) {
  String *string = new_string(token->location+1, token->length-2);
  add_string(string);
  Type *type = new_type_array(new_type_char(), string->length);

  Node *node = new_node_expr(ND_EXPR_STRING, type, token);
  node->string = string;
  return node;
}

//  ND_EXPR_FUNC_CALL,    // Function call
Node *new_node_func_call(char *name, int len, Node *args, Token *token) {
  Function *func = find_function(name, len);
  if (!func)
    error_at(token, "called undefined function");

  Node *node = new_node_expr(ND_EXPR_FUNC_CALL, func->type, token);
  node->func_name = name;
  node->func_args = args;
  node->func_namelen = len;

  return node;
}

//  ND_EXPR_ADDR,         // & ...
Node *new_node_addr(Node *unary_node, Token *token) {
  Node *node = new_node_expr(ND_EXPR_ADDR, new_type_pointer(unary_node->type), token);
  node->left = unary_node;
  return node;
}

//  ND_EXPR_DEREF,        // * ...
Node *new_node_deref(Node *unary_node, Token *token) {
  Node *node = new_node_expr(ND_EXPR_DEREF, unary_node->type->base, token);
  node->left = unary_node;
  return node;
}

//  ND_EXPR_NOT,          // ! ...
Node *new_node_not(Node *left, Token *token) {
  Node *node = new_node_expr(ND_EXPR_NOT, new_type_int(), token);
  node->left = left;
  return node;
}

//  ND_EXPR_BIT_NOT,      // ~ ...
Node *new_node_bit_not(Node *left, Token *token) {
  Node *node = new_node_expr(ND_EXPR_BIT_NOT, left->type, token);
  node->left = left;
  return node;
}

//  ND_EXPR_PRE_INC,      // ++ ...
Node *new_node_pre_increment(Node *left, Token *token) {
  Node *node = new_node_expr(ND_EXPR_PRE_INC, left->type, token);
  node->left = left;
  return node;
}
//  ND_EXPR_PRE_DEC,      // -- ...
Node *new_node_pre_decrement(Node *left, Token *token) {
  Node *node = new_node_expr(ND_EXPR_PRE_DEC, left->type, token);
  node->left = left;
  return node;
}

//  ND_EXPR_POST_INC,     // ++ ...
Node *new_node_post_increment(Node *left, Token *token) {
  Node *node = new_node_expr(ND_EXPR_POST_INC, left->type, token);
  node->left = left;
  return node;
}

//  ND_EXPR_POST_DEC,     // -- ...
Node *new_node_post_decrement(Node *left, Token *token) {
  Node *node = new_node_expr(ND_EXPR_POST_DEC, left->type, token);
  node->left = left;
  return node;
}

//  ND_EXPR_TYPE_CAST     // (type)  ...
Node *new_node_cast(Node *left, Type *type, Token *token) {
  Node *node = new_node_expr(ND_EXPR_TYPE_CAST, type, token);
  node->left = left;
  return node;
}


//  ND_EXPR_ASSIGN,       // =
Node *new_node_assign(Node *left, Node *right, Token *token) {
  Node *node = new_node_op2(ND_EXPR_ASSIGN, left, right, token);
  node->type = left->type;
  return node;
}

//  ND_EXPR_ASSIGN_ADD,   // +=
//  ND_EXPR_ASSIGN_PTR_ADD, // (pointer) +=
Node *new_node_assign_add(Node *left, Node *right, Token *token) {
  if (is_ptr_or_arr(left->type))
    return new_node_op2(ND_EXPR_ASSIGN_PTR_ADD, left, right, token);
  return new_node_op2(ND_EXPR_ASSIGN_ADD, left, right, token);
}

//  ND_EXPR_ASSIGN_SUB,   // -=
//  ND_EXPR_ASSIGN_PTR_SUB, // (pointer) -=
Node *new_node_assign_sub(Node *left, Node *right, Token *token) {
  if (is_ptr_or_arr(left->type))
    return new_node_op2(ND_EXPR_ASSIGN_PTR_SUB, left, right, token);
  return new_node_op2(ND_EXPR_ASSIGN_SUB, left, right, token);
}

//  ND_EXPR_ASSIGN_MUL,   // *=
Node *new_node_assign_mul(Node *left, Node *right, Token *token) {
  return new_node_op2(ND_EXPR_ASSIGN_MUL, left, right, token);
}
//  ND_EXPR_ASSIGN_DIV,   // /=
Node *new_node_assign_div(Node *left, Node *right, Token *token) {
  return new_node_op2(ND_EXPR_ASSIGN_DIV, left, right, token);
}
//  ND_EXPR_ASSIGN_MOD,   // %=
Node *new_node_assign_mod(Node *left, Node *right, Token *token) {
  return new_node_op2(ND_EXPR_ASSIGN_MOD, left, right, token);
}

// ND_EXPR_ASSIGN_SHIFT_LEFT, // <<=
Node *new_node_assign_shift_left(Node *left, Node *right, Token *token) {
  return new_node_op2(ND_EXPR_ASSIGN_SHIFT_LEFT, left, right, token);
}

// ND_EXPR_ASSIGN_SHIFT_RIGHT, // >>=
Node *new_node_assign_shift_right(Node *left, Node *right, Token *token) {
  return new_node_op2(ND_EXPR_ASSIGN_SHIFT_RIGHT, left, right, token);
}

//  ND_EXPR_ADD,          // +
//  ND_EXPR_PTR_ADD,      // (pointer) + (int)
Node *new_node_add(Node *left, Node *right, Token *token) {
  if (is_ptr_or_arr(left->type))
    return new_node_op2(ND_EXPR_PTR_ADD, left, right, token);
  if (is_ptr_or_arr(right->type))
    return new_node_op2(ND_EXPR_PTR_ADD, right, left, token);
  return new_node_op2(ND_EXPR_ADD, left, right, token);
}

//  ND_EXPR_SUB,          // -
//  ND_EXPR_PTR_SUB,      // (pointer) - (int)
//  ND_EXPR_PTR_DIFF,     // (pointer) - (pointer)
Node *new_node_sub(Node *left, Node *right, Token *token) {
  if (is_ptr_or_arr(left->type) && is_ptr_or_arr(right->type)) {
    Node *node = new_node_op2(ND_EXPR_PTR_DIFF, left, right, token);
    node->type = new_type_int();
    return node;
  }
  if (is_ptr_or_arr(left->type))
    return new_node_op2(ND_EXPR_PTR_SUB, left, right, token);
  if (is_ptr_or_arr(right->type))
    return new_node_op2(ND_EXPR_PTR_SUB, right, left, token);

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
    error_at(token, "refered undefined member of struct");
  
  Node *node = new_node_expr(ND_EXPR_MEMBER, member->type, token);
  node->left = parent;
  node->member = member;
  return node;
}

//  ND_EXPR_COMMA,        // ... , ...
Node *new_node_comma(Node *left, Node *right, Token *token) {
  Node *node = new_node_expr(ND_EXPR_COMMA, right->type, token);
  node->left = left;
  node->right = right;
  return node;
}

//  ND_EXPR_BIT_OR,       // ... | ...
Node *new_node_bit_or(Node *left, Node *right, Token *token) {
  return new_node_op2(ND_EXPR_BIT_OR, left, right, token);
}

//  ND_EXPR_BIT_XOR,      // ... ^ ...
Node *new_node_bit_xor(Node *left, Node *right, Token *token) {
  return new_node_op2(ND_EXPR_BIT_XOR, left, right, token);
}

//  ND_EXPR_BIT_AND,      // ... & ...
Node *new_node_bit_and(Node *left, Node *right, Token *token) {
  return new_node_op2(ND_EXPR_BIT_AND, left, right, token);
}

//  ND_EXPR_LOG_OR,       // ... || ...
Node *new_node_logical_or(Node *left, Node *right, Token *token) {
  Node *node = new_node_expr(ND_EXPR_LOG_OR, new_type_int(), token);
  node->left = left;
  node->right = right;
  return node;
}

//  ND_EXPR_LOG_AND,      // ... && ...
Node *new_node_logical_and(Node *left, Node *right, Token *token) {
  Node *node = new_node_expr(ND_EXPR_LOG_AND, new_type_int(), token);
  node->left = left;
  node->right = right;
  return node;
}

//  ND_EXPR_SHIFT_LEFT,   // ... << ...
Node *new_node_shift_left(Node *left, Node *right, Token *token) {
  return new_node_op2(ND_EXPR_SHIFT_LEFT, left, right, token);
}

//  ND_EXPR_SHIFT_RIGHT,  // ... >> ...
Node *new_node_shift_right(Node *left, Node *right, Token *token) {
  return new_node_op2(ND_EXPR_SHIFT_RIGHT, left, right, token);
}

//  ND_EXPR_TERNARY,      // ... ? ... : ...
Node *new_node_ternary(Node *cond, Node *left, Node *right, Token *token) {
  Node *node = new_node_op2(ND_EXPR_TERNARY, left, right, token);
  node->cond = cond;
  return node;
}

//  ND_EXPR_WITH_STMTS,   // ({ ...; ... }) GNU statement expression extention
//  created in src/parse/expr.c

//  ND_STMT_BLOCK,        // { ... }
Node *new_node_block(Node *body, Token *token) {
  Node *node = new_node_stmt(ND_STMT_BLOCK, token);
  node->body = body;
  return node;
}

//  ND_STMT_IF,           // if
Node *new_node_if(Node *cond, Node *then, Node *els, Token *token) {
  Node *node = new_node_stmt(ND_STMT_IF, token);
  node->cond = cond;
  node->then = then;
  node->els = els;
  return node;
}

//  ND_STMT_SWITCH,       // switch { ... }
Node *new_node_switch(Node *cond, Node *cases, Node *body, bool have_default, Token *token) {
  Node *node = new_node_stmt(ND_STMT_SWITCH, token);
  node->cond = cond;
  node->cases = cases;
  node->body = body;
  node->have_default = have_default;
  return node;
}

//  ND_STMT_WHILE,        // while
Node *new_node_while(Node *cond, Node *then, Token *token) {
  Node *node = new_node_stmt(ND_STMT_WHILE, token);
  node->cond = cond;
  node->then = then;
  return node;
}

//  ND_STMT_FOR,          // for
Node *new_node_for(Node *init, Node *cond, Node* increment, Node *then, Token *token) {
  Node *node = new_node_stmt(ND_STMT_FOR, token);
  node->init = init;
  node->increment = increment;
  node->cond = cond;
  node->then = then;
  return node;
}

//  ND_STMT_RETURN,       // return
Node *new_node_return(Node *left, Token *token) {
  Node *node = new_node_stmt(ND_STMT_RETURN, token);
  node->left = left;
  return node;
}

//  ND_STMT_WITH_EXPR,    // ... ;
Node *new_node_expr_stmt(Node *expr_node, Token *token) {
  Node *node = new_node_stmt(ND_STMT_WITH_EXPR, token);
  node->left = expr_node;
  return node;
}

//  ND_STMT_VAR_INIT,     // initialize variable
Node *new_node_var_init(Token *token, Var *var, char *var_inits, int var_inits_size) {
  Node *node = new_node_stmt(ND_STMT_VAR_INIT, token);
  node->var = var;
  node->var_inits = var_inits;
  node->var_inits_size = var_inits_size;
  return node;
}

//  ND_STMT_CONTINUE,     // continue;
Node *new_node_continue(Token *token) {
  return new_node_stmt(ND_STMT_CONTINUE, token);
}

//  ND_STMT_BREAK,        // break;
Node *new_node_break(Token *token) {
  return new_node_stmt(ND_STMT_BREAK, token);
}

//  ND_LABEL_CASE,        // case expr: in switch statement
Node *new_node_case(Token *token, int case_num) {
  Node *node = new_node_stmt(ND_LABEL_CASE, token);
  node->case_num = case_num;
  return node;
}

//  ND_LABEL_DEFAULT,     // default: in switch statement
Node *new_node_default(Token *token) {
  return new_node_stmt(ND_LABEL_DEFAULT, token);
}
