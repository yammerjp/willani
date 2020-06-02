#include "willani.h"

FILE *lvar_logfile;
FILE *parse_logfile;

static void print_type(FILE *file, Type *type) {
  if (type->ptr_to) {
    print_type(file, type->ptr_to);
  }
  switch (type->kind) {
  case TYPE_INT:
    fprintf(file, "int");
    return;
  case TYPE_PTR:
    fprintf(file, "*");
    return;
  case TYPE_ARRAY:
    fprintf(file, "[%d]", type_size(type) / type_size(type->ptr_to));
    return;
  default : error("unexpected Type.kind");
  }
}

static void print_node_kind(FILE *file, Node *node) {
  switch (node->kind) {
    case ND_ADD:          fprintf(file, "+"); return;
    case ND_SUB:          fprintf(file, "-"); return;
    case ND_MUL:          fprintf(file, "*"); return;
    case ND_DIV:          fprintf(file, "/"); return;
    case ND_EQ:           fprintf(file, "=="); return;
    case ND_NE:           fprintf(file, "!="); return;
    case ND_LT:           fprintf(file, "<"); return;
    case ND_LE:           fprintf(file, "<="); return;
    case ND_ASSIGN:       fprintf(file, "="); return;
    case ND_LVAR:         fprintf(file, "Variable"); return;
    case ND_NUM:          fprintf(file, "Integer"); return;
    case ND_ADDR:         fprintf(file, "&"); return;
    case ND_DEREF:        fprintf(file, "*"); return;
    case ND_RETURN:       fprintf(file, "return"); return;
    case ND_IF:           fprintf(file, "if"); return;
    case ND_WHILE:        fprintf(file, "while"); return;
    case ND_FOR:          fprintf(file, "for"); return;
    case ND_BLOCK:        fprintf(file, "{}"); return;
    case ND_FUNC_CALL:    fprintf(file, "func call"); return;
    case ND_EXPR_STMT:    fprintf(file, ";"); return;
    case ND_DECLARE_LVAR: print_type(file, node->lvar->type);
                          fprintf(file, " %.*s", node->lvar->length, node->lvar->name);
                          return;
    default : error("unexpected node->kind");
  }
}

void print_node(FILE *file, Node *node) {
  if (node->kind == ND_NUM) {
    fprintf(file, "%ld\n",node->value);
    return;
  }

  if (node->kind == ND_LVAR) {
    fprintf(file, "%.*s\n",node->lvar->length, node->lvar->name);
    return;
  }
  if (node->kind == ND_FUNC_CALL) {
    fprintf(file, "%.*s\n",node->fncl->length, node->fncl->name);
    return;
  }
  print_node_kind(file, node);
  fprintf(file, "\n");
}

static void parse_log_node(Node *node, int depth) {
  if (node == NULL) {
    return;
  }
  fprintf(parse_logfile, "%*s",depth*2, "");
  if (node->type) {
    fprintf(parse_logfile, "<");
    print_type(parse_logfile, node->type);
    fprintf(parse_logfile, "> ");
  }
  print_node(parse_logfile, node);
}

static void parse_log_nodes(Node *node, int depth) {
  if (node == NULL) {
    return;
  }
  parse_log_node(node, depth);

  parse_log_nodes(node->left, depth+1);
  parse_log_nodes(node->right, depth+1);
  parse_log_nodes(node->init, depth+1);
  parse_log_nodes(node->cond, depth+1);
  parse_log_nodes(node->increment, depth+1);
  parse_log_nodes(node->then, depth+1);
  parse_log_nodes(node->els, depth+1);
  parse_log_nodes(node->body, depth+1);
  parse_log_nodes(node->next, depth);
}

static void parse_lvar(Function *func) {
  Var *lvar = func->lvar;
  char *func_name = func->name;
  int func_name_len = func->namelen;
  Var *args =  func->args;

  fprintf(lvar_logfile, "%.*s:\n", func_name_len, func_name);
  for( Var *cur = lvar; cur; cur = cur->next) {
    fprintf(lvar_logfile, "  %.*s (offset: %d)", cur->length, cur->name, cur->offset);
    if (cur == args) {
      fprintf(lvar_logfile, " (argument)");
      args = cur->next;
    }

    fprintf(lvar_logfile, "\n");
  }
}

void parse_log_func(Function *func) {
  parse_lvar(func);
  parse_log_nodes(func->node, 0);
}

void parse_log(Function *func) {
  lvar_logfile = fopen("lvar.log","w");
  if (lvar_logfile == NULL) {
    error("fail to open lvar.log");
  }
  parse_logfile = fopen("parse.log","w");
  if (parse_logfile == NULL) {
    error("fail to open parse.log");
  }

  for (Function *current = func; current; current = current->next) {
    parse_log_func(current);
  }
 
  fclose(parse_logfile);
  fclose(lvar_logfile);
}
