#include "parse.h"

FILE *parse_logfile;

static void print_type(FILE *file, Type *type) {
  if (type->base)
    print_type(file, type->base);

  switch (type->kind) {
  case TYPE_VOID:
    fprintf(file, "void");
    return;
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
  case TYPE_ENUM:
    fprintf(file, "enum");
    return;
  case TYPE_ARRAY:
    fprintf(file, "[%d]", type->size / type->base->size);
    return;
  default : error("unexpected Type.kind");
  }
}

void print_node(FILE *file, Node *node) {
  switch (node->kind) {
    // single operator
    case ND_EXPR_VAR:             fprintf(file, "%.*s",node->var->namelen, node->var->name); break;
    case ND_EXPR_NUM:             fprintf(file, "%ld",node->value); break;
    case ND_EXPR_STRING:          fprintf(file, "%.*s",node->token->length, node->token->location); break;
    case ND_EXPR_FUNC_CALL:       fprintf(file, "%.*s()",node->func_namelen, node->func_name); break;
    case ND_EXPR_ADDR:            fprintf(file, "&"); break;
    case ND_EXPR_DEREF:           fprintf(file, "*"); break;
    case ND_EXPR_NOT:             fprintf(file, "!"); break;
    case ND_EXPR_BIT_NOT:         fprintf(file, "~"); break;
    case ND_EXPR_PRE_INC:         fprintf(file, "++ ..."); break;
    case ND_EXPR_PRE_DEC:         fprintf(file, "-- ..."); break;
    case ND_EXPR_POST_INC:        fprintf(file, "... ++"); break;
    case ND_EXPR_POST_DEC:        fprintf(file, "... --"); break;
    // dual operator
    case ND_EXPR_ASSIGN:          fprintf(file, "="); break;
    case ND_EXPR_ASSIGN_ADD:      fprintf(file, "+="); break;
    case ND_EXPR_ASSIGN_PTR_ADD:  fprintf(file, "(pointer) +="); break;
    case ND_EXPR_ASSIGN_SUB:      fprintf(file, "-="); break;
    case ND_EXPR_ASSIGN_PTR_SUB:  fprintf(file, "(pointer) -="); break;
    case ND_EXPR_ASSIGN_MUL:      fprintf(file, "*="); break;
    case ND_EXPR_ASSIGN_DIV:      fprintf(file, "/="); break;
    case ND_EXPR_ASSIGN_MOD:      fprintf(file, "%%="); break;
    case ND_EXPR_ADD:             fprintf(file, "+"); break;
    case ND_EXPR_PTR_ADD:         fprintf(file, "+ (pointer)"); break;
    case ND_EXPR_SUB:             fprintf(file, "-"); break;
    case ND_EXPR_PTR_SUB:         fprintf(file, "- (pointer)"); break;
    case ND_EXPR_PTR_DIFF:        fprintf(file, "- (pointers diff)"); break;
    case ND_EXPR_MUL:             fprintf(file, "*"); break;
    case ND_EXPR_DIV:             fprintf(file, "/"); break;
    case ND_EXPR_MOD:             fprintf(file, "%%"); break;
    case ND_EXPR_EQ:              fprintf(file, "=="); break;
    case ND_EXPR_NE:              fprintf(file, "!="); break;
    case ND_EXPR_LESS_THAN:       fprintf(file, "<"); break;
    case ND_EXPR_LESS_EQ:         fprintf(file, "<="); break;
    case ND_EXPR_MEMBER:          fprintf(file, ".%.*s", node->member->namelen, node->member->name); break;
    case ND_EXPR_COMMA:           fprintf(file, ","); break;
    case ND_EXPR_BIT_OR:          fprintf(file, "|"); break;
    case ND_EXPR_BIT_XOR:         fprintf(file, "^"); break;
    case ND_EXPR_BIT_AND:         fprintf(file, "&"); break;
    case ND_EXPR_LOG_OR:          fprintf(file, "||"); break;
    case ND_EXPR_LOG_AND:         fprintf(file, "&&"); break;
    // multiple operator
    case ND_EXPR_TERNARY:         fprintf(file, "?:"); break;
    case ND_EXPR_WITH_STMTS:      fprintf(file, "({})"); break;
    // statement
    case ND_STMT_BLOCK:           fprintf(file, "{}"); break;
    case ND_STMT_IF:              fprintf(file, "if"); break;
    case ND_STMT_SWITCH:          fprintf(file, "switch"); break;
    case ND_STMT_WHILE:           fprintf(file, "while"); break;
    case ND_STMT_FOR:             fprintf(file, "for"); break;
    case ND_STMT_RETURN:          fprintf(file, "return"); break;
    case ND_STMT_WITH_EXPR:       fprintf(file, ";"); break;
    case ND_STMT_CONTINUE:        fprintf(file, "continue"); break;
    case ND_STMT_BREAK:           fprintf(file, "break"); break;
    case ND_LABEL_CASE:           fprintf(file, "default"); break;
    case ND_LABEL_DEFAULT:        fprintf(file, "case %dth", node->case_num); break;
    default : error_at(node->token, "unexpected node->kind");
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
