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

void print_node(FILE *file, Node *node) {
  if (node->kind == ND_NUM) {
    fprintf(file, "%ld\n",node->value);
    return;
  }

  if (node->kind == ND_VAR) {
    fprintf(file, "%.*s\n",node->lvar->length, node->lvar->name);
    return;
  }
  if (node->kind == ND_FUNC_CALL) {
    fprintf(file, "%.*s\n",node->fncl->length, node->fncl->name);
    return;
  }
  fprintf(file, "%s\n", node_kind_str(node));
}

static void parse_log_node(FILE *logfile, Node *node, int depth) {
  if (node == NULL) {
    return;
  }

  fprintf(logfile, "%*s",depth*2, "");

  print_node(logfile, node);
}

static void parse_log_nodes(FILE *logfile, Node *node, int depth) {
  if (node == NULL) {
    return;
  }
  if (node->kind == ND_BLOCK) {
    parse_log_node(logfile, node, depth);
    parse_log_nodes(logfile, node->body, depth+1);
    return;
  }
  if (node->kind == ND_FOR) {
    parse_log_node(logfile, node, depth);
    parse_log_nodes(logfile, node->init, depth+1);
    parse_log_nodes(logfile, node->cond, depth+1);
    parse_log_nodes(logfile, node->increment, depth+1);
    parse_log_nodes(logfile, node->then, depth+1);
    return;
  }
  parse_log_nodes(logfile, node->left, depth+1);
  parse_log_nodes(logfile, node->cond, depth+1);
  parse_log_node(logfile, node, depth);
  parse_log_nodes(logfile, node->right, depth+1);
  parse_log_nodes(logfile, node->then, depth+1);

  parse_log_nodes(logfile, node->next, depth);
}

static void parse_lvar(LVar *lvar, char *func_name, int func_name_len) {
  FILE *logfile;
  logfile = fopen("lvar.log","w");
  if (logfile == NULL) {
    error("fail to open lvar.log");
  }

  fprintf(logfile, "%.*s:\n", func_name_len, func_name);
  for( LVar *cur = lvar; cur; cur = cur->next) {
    fprintf(logfile, "  %.*s (offset: %d\n)", cur->length, cur->name, cur->offset);
  }
  fclose(logfile);
}

void parse_log(Function *func) {
  parse_lvar(func->lvar, func->name, func->namelen);

  FILE *logfile;
  logfile = fopen("parse.log","w");
  if (logfile == NULL) {
    error("fail to open parse.log");
  }
  Node *node = func->node;

  parse_log_nodes(logfile, node, 0);

  fclose(logfile);
}
