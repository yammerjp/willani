#include "parse.h"

static FILE *parse_logfile;

static void indent(int length);
static void print_key(char *s);

static void print_ast_member(Member *member, int idt);
static void print_ast_members(Member *members, int idt);

static char *type_kind_string(TypeKind kind);
static void print_ast_type(Type *type, int idt);

static void print_ast_var(Var *var, int idt);
static void print_ast_vars(Var *vars, int idt);

static char *token_kind_string(TokenKind kind);
static void print_ast_token(Token *token, int idt);

static char *node_kind_string(NodeKind kind);
static void print_ast_node(Node *node, int idt);
static void print_ast_nodes(Node *node, int idt);

static void print_ast_function(Function *func, int idt);
static void print_ast_functions(Function *funcs, int idt);


// >> detect type reference loop
typedef struct TypeStack TypeStack;
struct TypeStack {
  Type *type;
  TypeStack *parent;
};
TypeStack *stack = NULL;

void push(Type *type) {
  TypeStack *s = calloc(1, sizeof(TypeStack));
  s->type = type;
  s->parent = stack;
  stack = s;
}
void drop() {
  stack = stack->parent;
}

int find(Type *type) {
  int i =0;
  for (TypeStack *s = stack; s; s = s->parent) {
    i++;
    if(s->type == type)
      return i;
  }
  return 0;
}
// << detect type reference loop

static void findent(FILE *file, int length) {
  if (length)
    fprintf(file, "%*s", length, "");
}

static void indent(int length) {
  findent(parse_logfile, length);
}

static void fprint_key(FILE *file, char *s) {
  fprintf(file, "\"%s\": ", s);
}
static void print_key(char *s) {
  fprint_key(parse_logfile, s);
}

static void print_ast_member(Member *member, int idt) {
  fprintf(parse_logfile, "{\n");

  indent(idt+2); print_key("struct");   fprintf(parse_logfile, "\"Member\",\n");
  indent(idt+2); print_key("name");     fprintf(parse_logfile, "\"%.*s\",\n", member->namelen, member->name);
  indent(idt+2); print_key("offset");   fprintf(parse_logfile, "%d,\n", member->offset);
  indent(idt+2); print_key("type");     print_ast_type(member->type, idt+2);

  fprintf(parse_logfile, "\n");
  indent(idt); fprintf(parse_logfile, "}");
}

static void print_ast_members(Member *members, int idt) {
  fprintf(parse_logfile, "[");
  bool is_head = true;
  for (Member *mem = members; mem; mem = mem->next) {
    fprintf(parse_logfile, is_head ? "\n" : ",\n");
    is_head = false;

    indent(idt+2); print_ast_member(mem, idt+2);
  }
  fprintf(parse_logfile, "\n");
  indent(idt); fprintf(parse_logfile, "]");

}

static char *type_kind_string(TypeKind kind) {
  switch (kind) {
    case TYPE_VOID:   return "TYPE_VOID";
    case TYPE_LONG:   return "TYPE_LONG";
    case TYPE_INT:    return "TYPE_INT";
    case TYPE_CHAR:   return "TYPE_CHAR";
    case TYPE_BOOL:   return "TYPE_BOOL";
    case TYPE_PTR:    return "TYPE_PTR";
    case TYPE_ARRAY:  return "TYPE_ARRAY";
    case TYPE_STRUCT: return "TYPE_STRUCT";
    case TYPE_ENUM:   return "TYPE_ENUM";
    default : error("unknown TypeKind");
  }
}

static void print_ast_type(Type *type, int idt) {
  int loop_type_generation_diff = find(type);
  if (loop_type_generation_diff) {
    fprintf(parse_logfile, "{\n");
    indent(idt+2); print_key("struct");           fprintf(parse_logfile, "\"Type(Reference)\",\n");
    indent(idt+2); print_key("generations_diff"); fprintf(parse_logfile, "%d\n", loop_type_generation_diff);
    indent(idt); fprintf(parse_logfile, "}");
    return;
  }
  push(type);

  fprintf(parse_logfile, "{\n");

  indent(idt+2); print_key("struct");           fprintf(parse_logfile, "\"Type\",\n");
  indent(idt+2); print_key("kind");             fprintf(parse_logfile, "\"%s\",\n", type_kind_string(type->kind));
  indent(idt+2); print_key("size");             fprintf(parse_logfile, "%d,\n", type->size);
  indent(idt+2); print_key("array_length");     fprintf(parse_logfile, "%d,\n", type->array_length);

  indent(idt+2); print_key("is_static");        fprintf(parse_logfile, type->is_static ? "true,\n" : "false,\n");
  indent(idt+2); print_key("is_extern");        fprintf(parse_logfile, type->is_extern ? "true,\n" : "false,\n");
  indent(idt+2); print_key("is_const");         fprintf(parse_logfile, type->is_const ? "true,\n" : "false,\n");
  indent(idt+2); print_key("undefined_member"); fprintf(parse_logfile, type->undefined_member ? "true" : "false");

  if (type->base) {
    fprintf(parse_logfile, ",\n");
    indent(idt+2); print_key("base");
      print_ast_type(type->base, idt+2);
  }
  if (type->members) {
    fprintf(parse_logfile, ",\n");
    indent(idt+2); print_key("base");
      print_ast_members(type->members, idt+2);
  }

  fprintf(parse_logfile, "\n");
  indent(idt); fprintf(parse_logfile, "}");

  drop();
}

static void print_ast_var(Var *var, int idt) {
  fprintf(parse_logfile, "{\n");

  indent(idt+2); print_key("struct");     fprintf(parse_logfile, "\"Var\",\n");
  indent(idt+2); print_key("name");       fprintf(parse_logfile, "\"%.*s\",\n", var->namelen, var->name);
  indent(idt+2); print_key("offset");     fprintf(parse_logfile, "%d,\n", var->offset);
  indent(idt+2); print_key("is_global");  fprintf(parse_logfile, var->is_global ? "true,\n" : "false,\n");
  indent(idt+2); print_key("is_extern");  fprintf(parse_logfile, var->is_extern ? "true,\n" : "false,\n");
  indent(idt+2); print_key("init_size");  fprintf(parse_logfile, "%d", var->init_size);
  if (var->init_size) {
    fprintf(parse_logfile, ",\n");
    indent(idt+2); print_key("init_values"); fprintf(parse_logfile, "[");
    for (int i=0; i < var->init_size; i++)
      fprintf(parse_logfile, "%s %d", i==0 ? "" : ",", var->init_values[i]);
    fprintf(parse_logfile, " ]");
  }
  fprintf(parse_logfile, ",\n");
  indent(idt+2); print_key("type");
    print_ast_type(var->type, idt+2);

  fprintf(parse_logfile, "\n");
  indent(idt); fprintf(parse_logfile, "}");
}

static void print_ast_vars(Var *vars, int idt) {
  fprintf(parse_logfile, "[");
  bool is_head = true;
  for (Var *var = vars; var; var=var->next) {
    fprintf(parse_logfile, is_head ? "\n" : ",\n");
    is_head = false;

    indent(idt+2); print_ast_var(var, idt+2);
  }
  fprintf(parse_logfile, "\n");
  indent(idt); fprintf(parse_logfile, "]");
}
static char *node_kind_string(NodeKind kind) {
  switch (kind) {
    case ND_EXPR_VAR:             return "ND_EXPR_VAR";
    case ND_EXPR_NUM:             return "ND_EXPR_NUM";
    case ND_EXPR_STRING:          return "ND_EXPR_STRING";
    case ND_EXPR_FUNC_CALL:       return "ND_EXPR_FUNC_CALL";
    case ND_EXPR_ADDR:            return "ND_EXPR_ADDR";
    case ND_EXPR_DEREF:           return "ND_EXPR_DEREF";
    case ND_EXPR_NOT:             return "ND_EXPR_NOT";
    case ND_EXPR_BIT_NOT:         return "ND_EXPR_BIT_NOT";
    case ND_EXPR_PRE_INC:         return "ND_EXPR_PRE_INC";
    case ND_EXPR_PRE_DEC:         return "ND_EXPR_PRE_DEC";
    case ND_EXPR_POST_INC:        return "ND_EXPR_POST_INC";
    case ND_EXPR_POST_DEC:        return "ND_EXPR_POST_DEC";
    case ND_EXPR_TYPE_CAST:       return "ND_EXPR_TYPE_CAST";
    case ND_EXPR_ASSIGN:          return "ND_EXPR_ASSIGN";
    case ND_EXPR_ASSIGN_ADD:      return "ND_EXPR_ASSIGN_ADD";
    case ND_EXPR_ASSIGN_PTR_ADD:  return "ND_EXPR_ASSIGN_PTR_ADD";
    case ND_EXPR_ASSIGN_SUB:      return "ND_EXPR_ASSIGN_SUB";
    case ND_EXPR_ASSIGN_PTR_SUB:  return "ND_EXPR_ASSIGN_PTR_SUB";
    case ND_EXPR_ASSIGN_MUL:      return "ND_EXPR_ASSIGN_MUL";
    case ND_EXPR_ASSIGN_DIV:      return "ND_EXPR_ASSIGN_DIV";
    case ND_EXPR_ASSIGN_MOD:      return "ND_EXPR_ASSIGN_MOD";
    case ND_EXPR_ADD:             return "ND_EXPR_ADD";
    case ND_EXPR_PTR_ADD:         return "ND_EXPR_PTR_ADD";
    case ND_EXPR_SUB:             return "ND_EXPR_SUB";
    case ND_EXPR_PTR_SUB:         return "ND_EXPR_PTR_SUB";
    case ND_EXPR_PTR_DIFF:        return "ND_EXPR_PTR_DIFF";
    case ND_EXPR_MUL:             return "ND_EXPR_MUL";
    case ND_EXPR_DIV:             return "ND_EXPR_DIV";
    case ND_EXPR_MOD:             return "ND_EXPR_MOD";
    case ND_EXPR_EQ:              return "ND_EXPR_EQ";
    case ND_EXPR_NE:              return "ND_EXPR_NE";
    case ND_EXPR_LESS_THAN:       return "ND_EXPR_LESS_THAN";
    case ND_EXPR_LESS_EQ:         return "ND_EXPR_LESS_EQ";
    case ND_EXPR_MEMBER:          return "ND_EXPR_MEMBER";
    case ND_EXPR_COMMA:           return "ND_EXPR_COMMA";
    case ND_EXPR_BIT_OR:          return "ND_EXPR_BIT_OR";
    case ND_EXPR_BIT_XOR:         return "ND_EXPR_BIT_XOR";
    case ND_EXPR_BIT_AND:         return "ND_EXPR_BIT_AND";
    case ND_EXPR_LOG_OR:          return "ND_EXPR_LOG_OR";
    case ND_EXPR_LOG_AND:         return "ND_EXPR_LOG_AND";
    case ND_EXPR_ASSIGN_SHIFT_LEFT: return "ND_EXPR_ASSIGN_SHIFT_LEFT";
    case ND_EXPR_ASSIGN_SHIFT_RIGHT: return "ND_EXPR_ASSIGN_SHIFT_RIGHT";
    case ND_EXPR_SHIFT_LEFT:      return "ND_EXPR_SHIFT_LEFT";
    case ND_EXPR_SHIFT_RIGHT:     return "ND_EXPR_SHIFT_RIGHT";
    case ND_EXPR_TERNARY:         return "ND_EXPR_TERNARY";
    case ND_EXPR_WITH_STMTS:      return "ND_EXPR_WITH_STMTS";
    case ND_STMT_BLOCK:           return "ND_STMT_BLOCK";
    case ND_STMT_IF:              return "ND_STMT_IF";
    case ND_STMT_SWITCH:          return "ND_STMT_SWITCH";
    case ND_STMT_WHILE:           return "ND_STMT_WHILE";
    case ND_STMT_FOR:             return "ND_STMT_FOR";
    case ND_STMT_RETURN:          return "ND_STMT_RETURN";
    case ND_STMT_WITH_EXPR:       return "ND_STMT_WITH_EXPR";
    case ND_STMT_VAR_INIT:        return "ND_STMT_VAR_INIT";
    case ND_STMT_CONTINUE:        return "ND_STMT_CONTINUE";
    case ND_STMT_BREAK:           return "ND_STMT_BREAK";
    case ND_LABEL_CASE:           return "ND_LABEL_CASE";
    case ND_LABEL_DEFAULT:        return "ND_LABEL_DEFAULT";
    default : error("unknown NodeKind in node_kind_string()");
  }
}

static char *token_kind_string(TokenKind kind) {
  switch (kind) {
    case TK_RESERVED:         return "TK_RESERVED";
    case TK_IDENT:            return "TK_IDENT";
    case TK_STRING:           return "TK_STRING";
    case TK_CHAR:             return "TK_CHAR";
    case TK_STRUCT:           return "TK_STRUCT";
    case TK_NUM:              return "TK_NUM";
    case TK_EOF:              return "TK_EOF";
    case TK_PREPROCESS_BEGIN: return "TK_PREPROCESS_BEGIN";
    case TK_PREPROCESS_END:   return "TK_PREPROCESS_END";
    default : error("unknown TokenKind in token_kind_string()");
  }
}

static void print_ast_token(Token *token, int idt) {
  fprint_ast_token(parse_logfile, token, idt);
}

void fprint_ast_token(FILE *file, Token *token, int idt) {
  fprintf(file, "{ \n");

  findent(file, idt+2); fprint_key(file, "struct");           fprintf(file, "\"Token\", \n");
  findent(file, idt+2); fprint_key(file, "kind");             fprintf(file, "\"%s\", \n", token_kind_string(token->kind));

  findent(file, idt+2); fprint_key(file, "string");
  if (token->kind == TK_STRING)
    fprintf(file, "\"\\%.*s\\\"\", \n", token->length -1, token->location);
  else
    fprintf(file, "\"%.*s\", \n", token->length, token->location);

  if (!token->file)
    error("token->file is NULL");
  findent(file, idt+2); fprint_key(file, "filename");         fprintf(file, "\"%s\", \n", token->file->path);
  findent(file, idt+2); fprint_key(file, "prev_is_space");    fprintf(file, token->prev_is_space ? "true" : "false");

  fprintf(file, "\n");
  findent(file, idt);
  fprintf(file, "}");
}

static void print_ast_node(Node *node, int idt) {
  fprintf(parse_logfile, "{\n");

  indent(idt+2); print_key("struct");     fprintf(parse_logfile, "\"Node\",\n");
  indent(idt+2); print_key("kind");       fprintf(parse_logfile, "\"%s\",\n", node_kind_string(node->kind));
  indent(idt+2); print_key("token");      print_ast_token(node->token, idt+2); fprintf(parse_logfile, ",\n");
  indent(idt+2); print_key("value");      fprintf(parse_logfile, "%ld", node->value);

  if (node->var_inits_size) {
    fprintf(parse_logfile, ",\n");
    indent(idt+2); print_key("var_inits"); fprintf(parse_logfile, "[");
    bool is_head = true;
    for (int i=0; i < node->var_inits_size; i++) {
      fprintf(parse_logfile, is_head ? "" : ",");
      is_head = false;
      fprintf(parse_logfile, " %d", node->var_inits[i]);
    }
    fprintf(parse_logfile, " ]");
  }

  if (node->string) {
    fprintf(parse_logfile, ",\n");
    indent(idt+2); print_key("string");       fprintf(parse_logfile, "%.*s", node->token->length, node->token->location);
  }
  if (node->kind == ND_STMT_SWITCH) {
    fprintf(parse_logfile, ",\n");
    indent(idt+2); print_key("have_default"); fprintf(parse_logfile, node->have_default ? "true" : "false");
  }

  if (node->kind == ND_LABEL_CASE) {
    fprintf(parse_logfile, ",\n");
    indent(idt+2); print_key("case_num");       fprintf(parse_logfile, "%d", node->case_num);
  }

  if (node->var) {
    fprintf(parse_logfile, ",\n");
    indent(idt+2); print_key("var");
    print_ast_vars(node->var, idt+2);
  }
  if (node->member) {
    fprintf(parse_logfile, ",\n");
    indent(idt+2); print_key("member");
    print_ast_member(node->member, idt+2);
  }
  if (node->type) {
    fprintf(parse_logfile, ",\n");
    indent(idt+2); print_key("type");
    print_ast_type(node->type, idt+2);
  }

  if (node->func_name) {
    fprintf(parse_logfile, ",\n");
    indent(idt+2); print_key("func_name");       fprintf(parse_logfile, "\"%.*s\"", node->func_namelen, node->func_name);
  }
  if (node->func_args) {
    fprintf(parse_logfile, ",\n");
    indent(idt+2); print_key("func_args");
    print_ast_nodes(node->func_args, idt+2);
  }

  if (node->left) {
    fprintf(parse_logfile, ",\n");
    indent(idt+2); print_key("left");
    print_ast_node(node->left, idt+2);
  }
  if (node->right) {
    fprintf(parse_logfile, ",\n");
    indent(idt+2); print_key("right");
    print_ast_node(node->right, idt+2);
  }
  if (node->cond) {
    fprintf(parse_logfile, ",\n");
    indent(idt+2); print_key("cond");
    print_ast_node(node->cond, idt+2);
  }
  if (node->then) {
    fprintf(parse_logfile, ",\n");
    indent(idt+2); print_key("then");
    print_ast_node(node->then, idt+2);
  }
  if (node->els) {
    fprintf(parse_logfile, ",\n");
    indent(idt+2); print_key("els");
    print_ast_node(node->els, idt+2);
  }
  if (node->init) {
    fprintf(parse_logfile, ",\n");
    indent(idt+2); print_key("init");
    print_ast_node(node->init, idt+2);
  }
  if (node->increment) {
    fprintf(parse_logfile, ",\n");
    indent(idt+2); print_key("increment");
    print_ast_node(node->increment, idt+2);
  }
  if (node->body) {
    fprintf(parse_logfile, ",\n");
    indent(idt+2); print_key("body");
    print_ast_nodes(node->body, idt+2);
  }
  if (node->cases) {
    fprintf(parse_logfile, ",\n");
    indent(idt+2); print_key("cases");
    print_ast_nodes(node->cases, idt+2);
  }

  fprintf(parse_logfile, "\n");
  indent(idt); fprintf(parse_logfile, "}");
}

static void print_ast_nodes(Node *nodes, int idt) {
  fprintf(parse_logfile, "[");
  bool is_head = true;
  for (Node *node = nodes; node; node=node->next) {
    fprintf(parse_logfile, is_head ? "\n" : ",\n");
    is_head = false;

    indent(idt+2); print_ast_node(node, idt+2);
  }
  fprintf(parse_logfile, "\n");
  indent(idt); fprintf(parse_logfile, "]");
}

static void print_ast_function(Function *func, int idt) {
  fprintf(parse_logfile, "{\n");

  indent(idt+2); print_key("struct");     fprintf(parse_logfile, "\"Function\",\n");
  indent(idt+2); print_key("name");       fprintf(parse_logfile, "\"%.*s\",\n", func->namelen, func->name);
  indent(idt+2); print_key("var_byte");   fprintf(parse_logfile, "%d,\n", func->var_byte);
  indent(idt+2); print_key("definition"); fprintf(parse_logfile, func->definition ? "true,\n" : "false,\n");
  indent(idt+2); print_key("argc");       fprintf(parse_logfile, "%d", func->argc);
  if (func->argc) {
    fprintf(parse_logfile, ",\n");
    indent(idt+2); print_key("args");
    print_ast_vars(func->args, idt+2);
  }
  fprintf(parse_logfile, ",\n");
  indent(idt+2); print_key("type");
  print_ast_type(func->type, idt+2);

  if (func->node) {
    fprintf(parse_logfile, ",\n");
    indent(idt+2); print_key("node");
    print_ast_node(func->node, idt+2);
  }

  fprintf(parse_logfile, "\n");
  indent(idt); fprintf(parse_logfile, "}");
}

static void print_ast_functions(Function *funcs, int idt) {
  fprintf(parse_logfile, "[");
  bool is_head = true;
  for (Function *func = funcs; func; func = func->next) {
    fprintf(parse_logfile, is_head ? "\n" : ",\n");
    is_head = false;

    indent(idt+2); print_ast_function(func, idt+2);
  }
  fprintf(parse_logfile, "\n");
  indent(idt); fprintf(parse_logfile, "]\n");
}

void print_ast(Function *funcs) {
  parse_logfile = fopen("ast.json","w");
  if (!parse_logfile)
    error("fail to open ast.json");

  print_ast_functions(funcs, 0);

  fclose(parse_logfile);
}
