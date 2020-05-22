#include "willani.h"

char *node_kind_str(Node *node) {
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
    fprintf(logfile, "%c\n",node->name);
    return;
  }

  fprintf(logfile, "%s\n", node_kind_str(node));
}

static void print_nodes(FILE *logfile, Node *node, int depth) {
  if (node == NULL) {
    return;
  }
  if (node->kind == ND_NUM) {
    print_node(logfile, node, depth);
    return;
  }
  print_nodes(logfile, node->left, depth+1);
  print_node(logfile, node, depth);
  print_nodes(logfile, node->right, depth+1);
}

void parse_log(Node* head) {
  FILE *logfile;
  logfile = fopen("parse.log","w");
  if (logfile == NULL) {
    error("fail to open parse.log");
  }
  Node *node = head;
  print_nodes(logfile, node, 0);

  fclose(logfile);
}
