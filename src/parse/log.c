#include "parse.h"

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
    case ND_ADD:            fprintf(file, "+"); break;
    case ND_SUB:            fprintf(file, "-"); break;
    case ND_MUL:            fprintf(file, "*"); break;
    case ND_DIV:            fprintf(file, "/"); break;
    case ND_MOD:            fprintf(file, "%%"); break;
    case ND_EQ:             fprintf(file, "=="); break;
    case ND_NE:             fprintf(file, "!="); break;
    case ND_LT:             fprintf(file, "<"); break;
    case ND_LE:             fprintf(file, "<="); break;
    case ND_ASSIGN:         fprintf(file, "="); break;
    case ND_VAR:            fprintf(file, "%.*s",node->var->namelen, node->var->name); break;
    case ND_NUM:            fprintf(file, "%ld",node->value); break;
    case ND_STRING:         fprintf(file, "%.*s",node->token->length, node->token->location); break;
    case ND_ADDR:           fprintf(file, "&"); break;
    case ND_DEREF:          fprintf(file, "*"); break;
    case ND_MEMBER:         fprintf(file, ".%.*s", node->member->namelen, node->member->name); break;
    case ND_RETURN:         fprintf(file, "return"); break;
    case ND_IF:             fprintf(file, "if"); break;
    case ND_WHILE:          fprintf(file, "while"); break;
    case ND_FOR:            fprintf(file, "for"); break;
    case ND_BLOCK:          fprintf(file, "{}"); break;
    case ND_FUNC_CALL:      fprintf(file, "%.*s()",node->func_namelen, node->func_name); break;
    case ND_EXPR_STMT:      fprintf(file, ";"); break;
    case ND_CONTINUE_STMT:  fprintf(file, "continue"); break;
    case ND_BREAK_STMT:     fprintf(file, "break"); break;
    case ND_SWITCH_STMT:    fprintf(file, "switch"); break;
    case ND_DEFAULT_LABEL:  fprintf(file, "case %dth", node->case_num); break;
    case ND_CASE_LABEL:     fprintf(file, "default"); break;
    case ND_STMT_EXPR:      fprintf(file, "({})"); break;
    case ND_COMMA:          fprintf(file, ","); break;
    case ND_NOT:            fprintf(file, "!"); break;
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
  parse_log_nodes(node->cases, depth+1);
  parse_log_nodes(node->increment, depth+1);
  parse_log_nodes(node->then, depth+1);
  parse_log_nodes(node->els, depth+1);
  parse_log_nodes(node->body, depth+1);
  parse_log_nodes(node->func_args, depth+1);

  parse_log_nodes(node->next, depth);
}

void parse_log() {
  parse_logfile = fopen("parse.log","w");
  if (!parse_logfile)
    error("fail to open parse.log");

  for (Function *func = functions; func; func = func->next) {
    fprintf(parse_logfile, "%.*s: ", func->namelen, func->name);
    parse_log_nodes(func->node, 0);
  }
 
  fclose(parse_logfile);
}
