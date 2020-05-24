#include "willani.h"

static char *node_kind_str(Node *node) {
  switch (node->kind) {
    case ND_ADD: return("+");
    case ND_SUB: return("-");
    case ND_MUL: return("*");
    case ND_DIV: return("/");
    case ND_EQ:  return("==");
    case ND_NE:  return("!=");
    case ND_LT:  return("<");
    case ND_LE:  return("<=");
    case ND_ASSIGN: return("=");
    case ND_VAR: return("Variable");
    case ND_NUM: return("Integer");
    case ND_RETURN: return("return");
    case ND_IF: return("if");
    case ND_WHILE: return("while");
    case ND_FOR: return("for");
    case ND_BLOCK: return("{}");
    case ND_FUNC_CALL: return("func call");
    default : error("unexpected node->kind");
  }
}

static void print_node(FILE *logfile, Node *node, int depth) {
  if (node == NULL) {
    return;
  }

  fprintf(logfile, "%*s",depth*2, "");

  if (node->kind == ND_NUM) {
    fprintf(logfile, "%ld\n",node->value);
    return;
  }

  if (node->kind == ND_VAR) {
    fprintf(logfile, "%.*s\n",node->lvar->length, node->lvar->name);
    return;
  }
  if (node->kind == ND_FUNC_CALL) {
    fprintf(logfile, "%.*s\n",node->fncl->length, node->fncl->name);
  }


  fprintf(logfile, "%s\n", node_kind_str(node));
}

static void print_nodes(FILE *logfile, Node *node, int depth) {
  if (node == NULL) {
    return;
  }
  if (node->kind == ND_BLOCK) {
    print_node(logfile, node, depth);
    print_nodes(logfile, node->body, depth+1);
    return;
  }
  if (node->kind == ND_FOR) {
    print_node(logfile, node, depth);
    print_nodes(logfile, node->init, depth+1);
    print_nodes(logfile, node->cond, depth+1);
    print_nodes(logfile, node->increment, depth+1);
    print_nodes(logfile, node->then, depth+1);
    return;
  }
  print_nodes(logfile, node->left, depth+1);
  print_nodes(logfile, node->cond, depth+1);
  print_node(logfile, node, depth);
  print_nodes(logfile, node->right, depth+1);
  print_nodes(logfile, node->then, depth+1);

  print_nodes(logfile, node->next, depth);
}

static void parse_lvar(LVar *lvar) {
  FILE *logfile;
  logfile = fopen("lvar.log","w");
  if (logfile == NULL) {
    error("fail to open lvar.log");
  }
  for( LVar *cur = lvar; cur; cur = cur->next) {
    fprintf(logfile, "name: %.*s, offset: %d\n", cur->length, cur->name, cur->offset);
  }
  fclose(logfile);
}

void parse_log(Function *func) {
  parse_lvar(func->lvar);

  FILE *logfile;
  logfile = fopen("parse.log","w");
  if (logfile == NULL) {
    error("fail to open parse.log");
  }
  Node *node = func->node;

  print_nodes(logfile, node, 0);

  fclose(logfile);
}
