#include "parse.h"

FILE *var_logfile;
FILE *parse_logfile;

static void print_type(FILE *file, Type *type) {
  if (type->base)
    print_type(file, type->base);

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
  case TYPE_BOOL:
    fprintf(file, "bool");
    return;
  case TYPE_PTR:
    fprintf(file, "*");
    return;
  case TYPE_STRUCT:
    fprintf(file, "struct");
    return;
  case TYPE_ARRAY:
    fprintf(file, "[%d]", type->size / type->base->size);
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
    case ND_STRING:       fprintf(file, "%.*s",node->token->length, node->token->location); break;
    case ND_ADDR:         fprintf(file, "&"); break;
    case ND_DEREF:        fprintf(file, "*"); break;
    case ND_MEMBER:       fprintf(file, ".%.*s", node->member->namelen, node->member->name); break;
    case ND_RETURN:       fprintf(file, "return"); break;
    case ND_IF:           fprintf(file, "if"); break;
    case ND_WHILE:        fprintf(file, "while"); break;
    case ND_FOR:          fprintf(file, "for"); break;
    case ND_BLOCK:        fprintf(file, "{}"); break;
    case ND_FUNC_CALL:    fprintf(file, "%.*s()",node->func_namelen, node->func_name); break;
    case ND_EXPR_STMT:    fprintf(file, ";"); break;
    case ND_STMT_EXPR:    fprintf(file, "({})"); break;
    default : error_at(node->token->location, "unexpected node->kind");
  }

  fprintf(file, "    (line: %d", get_line_number(get_line_head(node->token->location)));
  fprintf(file, ", token: %.*s", node->token->length, node->token->location);
  if (node->type) {
    fprintf(file, ", type: ");
    print_type(parse_logfile, node->type);
  }
  fprintf(file, ")\n");
}

static void parse_log_nodes(Node *node, int depth) {
  if (!node)
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
  parse_log_nodes(node->func_args, depth+1);

  parse_log_nodes(node->next, depth);
}

static void parse_var_line(int depth, int size, int offset, int namelen, char *name, bool is_arg) {
  fprintf(var_logfile, "%*s", (depth+1)*2, "");
  fprintf(var_logfile, "size:%3d", size);
  fprintf(var_logfile, ", offset:%3d", offset);
  fprintf(var_logfile, ", name: %.*s ", namelen, name);
  fprintf(var_logfile, is_arg ? " (argument)" : "" );
  fprintf(var_logfile, "\n");
}

static void parse_members(Member *members, int depth) {
  if (!members)
    return;
  for (Member *cur = members; cur; cur = cur->next) {
    parse_var_line(depth+1, cur->type->size, cur->offset, cur->namelen, cur->name, false);
    parse_members(cur->type->members, depth+1);
  }
}

static void parse_vars(Var *vars, Var *args) {
  for (Var *cur = vars; cur; cur = cur->next) {
    if (cur->is_typedef)
      continue;
    parse_var_line(0, cur->type->size, cur->offset, cur->length, cur->name, cur==args);

    if (cur==args)
      args = cur->next;

    parse_members(cur->type->members, 0);
  }
}

void parse_log() {
  var_logfile = fopen("var.log","w");
  if (!var_logfile)
    error("fail to open var.log");

  parse_logfile = fopen("parse.log","w");
  if (!parse_logfile)
    error("fail to open parse.log");

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
