#include "parse.h"

FILE *var_logfile;
FILE *parse_logfile;

static void print_type(FILE *file, Type *type) {
  if (type->ptr_to) {
    print_type(file, type->ptr_to);
  }
  switch (type->kind) {
  case TYPE_LONG:
    fprintf(file, "long");
    return;
  case TYPE_INT:
    fprintf(file, "int");
    return;
  case TYPE_CHAR:
    fprintf(file, "char");
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

void print_node(FILE *file, Node *node) {
  switch (node->kind) {
    case ND_ADD:          fprintf(file, "+"); break;
    case ND_SUB:          fprintf(file, "-"); break;
    case ND_MUL:          fprintf(file, "*"); break;
    case ND_DIV:          fprintf(file, "/"); break;
    case ND_EQ:           fprintf(file, "=="); break;
    case ND_NE:           fprintf(file, "!="); break;
    case ND_LT:           fprintf(file, "<"); break;
    case ND_LE:           fprintf(file, "<="); break;
    case ND_ASSIGN:       fprintf(file, "="); break;
    case ND_GVAR:         fprintf(file, "%.*s",node->var->length, node->var->name); break;
    case ND_LVAR:         fprintf(file, "%.*s",node->var->length, node->var->name); break;
    case ND_NUM:          fprintf(file, "%ld",node->value); break;
    case ND_STRING:       fprintf(file, "%.*s",node->string->length, node->string->p); break;
    case ND_ADDR:         fprintf(file, "&"); break;
    case ND_DEREF:        fprintf(file, "*"); break;
    case ND_RETURN:       fprintf(file, "return"); break;
    case ND_IF:           fprintf(file, "if"); break;
    case ND_WHILE:        fprintf(file, "while"); break;
    case ND_FOR:          fprintf(file, "for"); break;
    case ND_BLOCK:        fprintf(file, "{}"); break;
    case ND_FUNC_CALL:    fprintf(file, "%.*s()",node->fncl->length, node->fncl->name); break;
    case ND_EXPR_STMT:    fprintf(file, ";"); break;
    default : error("unexpected node->kind");
  }

  fprintf(file, "    (line: %d", get_line_number(get_line_head(node->token)));
  fprintf(file, ", token: %.*s", node->token->length, node->token->location);
  if (node->type) {
    fprintf(file, ", type: ");
    print_type(parse_logfile, node->type);
  }
  fprintf(file, ")\n");
}

static void parse_log_nodes(Node *node, int depth) {
  if (node == NULL)
    return;

  fprintf(parse_logfile, "%*s",depth*2, "");
  print_node(parse_logfile, node);

  parse_log_nodes(node->left, depth+1);
  parse_log_nodes(node->right, depth+1);
  parse_log_nodes(node->init, depth+1);
  parse_log_nodes(node->cond, depth+1);
  parse_log_nodes(node->increment, depth+1);
  parse_log_nodes(node->then, depth+1);
  parse_log_nodes(node->els, depth+1);
  parse_log_nodes(node->body, depth+1);
  parse_log_nodes(node->next, depth);
  if (node->fncl)
    parse_log_nodes(node->fncl->args, depth+1);
}


static void parse_vars(Var *vars, Var *args) {
  for( Var *cur = vars; cur; cur = cur->next) {
    fprintf(var_logfile, "  size:%3d", type_size(cur->type));
    fprintf(var_logfile, ", offset:%3d", cur->offset);
    fprintf(var_logfile, ", name: %.*s ", cur->length, cur->name);

    if (cur == args) {
      fprintf(var_logfile, " (argument)");
      args = cur->next;
    }
    fprintf(var_logfile, "\n");
  }
}

void parse_log() {
  var_logfile = fopen("var.log","w");
  if (var_logfile == NULL) {
    error("fail to open var.log");
  }
  parse_logfile = fopen("parse.log","w");
  if (parse_logfile == NULL) {
    error("fail to open parse.log");
  }

  fprintf(var_logfile, ".global:\n");
  parse_vars(gvars, NULL);

  for (Function *cur = functions; cur; cur = cur->next) {
    fprintf(var_logfile, "%.*s:\n", cur->namelen, cur->name);
    parse_vars(cur->var, cur->args);

    fprintf(parse_logfile, "%.*s: ", cur->namelen, cur->name);
    parse_log_nodes(cur->node, 0);
  }
 
  fclose(parse_logfile);
  fclose(var_logfile);
}
