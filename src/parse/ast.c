#include "parse.h"

FILE *parse_logfile;
static void print_ast_type(Type *type, int idt);
static void print_ast_nodes(Node *nodes, int idt);
static void print_ast_token(Token *token, int idt);

static void print_log(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(parse_logfile, fmt, ap);
}

static void indent(int length) {
  if (length)
    print_log("%*s", length, "");
}

static void print_key(char *s) {
  print_log("\"%s\": ", s);
}

static void print_ast_member(Member *member, int idt) {
  print_log("{\n");

  indent(idt+2); print_key("struct");   print_log("\"Member\",\n");
  indent(idt+2); print_key("name");     print_log("\"%.*s\",\n", member->namelen, member->name);
  indent(idt+2); print_key("offset");   print_log("%d,\n", member->offset);
  indent(idt+2); print_key("type");     print_ast_type(member->type, idt+2);

  print_log("\n");
  indent(idt); print_log("}");
}

static void print_ast_members(Member *members, int idt) {
  print_log("[");
  bool is_head = true;
  for (Member *mem = members; mem; mem = mem->next) {
    print_log(is_head ? "\n" : ",\n");
    is_head = false;

    indent(idt+2); print_ast_member(mem, idt+2);
  }
  print_log("\n");
  indent(idt); print_log("]");

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
  print_log("{\n");

  indent(idt+2); print_key("struct");           print_log("\"Type\",\n");
  indent(idt+2); print_key("kind");             print_log("\"%s\",\n", type_kind_string(type->kind));
  indent(idt+2); print_key("size");             print_log("%d,\n", type->size);
  indent(idt+2); print_key("array_length");     print_log("%d,\n", type->array_length);

  indent(idt+2); print_key("is_static");        print_log(type->is_static ? "true,\n" : "false,\n");
  indent(idt+2); print_key("is_extern");        print_log(type->is_extern ? "true,\n" : "false,\n");
  indent(idt+2); print_key("is_const");         print_log(type->is_const ? "true,\n" : "false,\n");
  indent(idt+2); print_key("undefined_member"); print_log(type->undefined_member ? "true" : "false");

  if (type->base) {
    print_log(",\n");
    indent(idt+2); print_key("base");
      print_ast_type(type->base, idt+2);
  }
  if (type->members) {
    print_log(",\n");
    indent(idt+2); print_key("base");
      print_ast_members(type->members, idt+2);
  }

  print_log("\n");
  indent(idt); print_log("}");
}

static void print_ast_var(Var *var, int idt) {
  print_log("{\n");

  indent(idt+2); print_key("struct");     print_log("\"Var\",\n");
  indent(idt+2); print_key("name");       print_log("\"%.*s\",\n", var->namelen, var->name);
  indent(idt+2); print_key("offset");     print_log("%d,\n", var->offset);
  indent(idt+2); print_key("is_global");  print_log(var->is_global ? "true,\n" : "false,\n");
  indent(idt+2); print_key("is_extern");  print_log(var->is_extern ? "true,\n" : "false,\n");
  indent(idt+2); print_key("init_size");  print_log("%d", var->init_size);
  if (var->init_size) {
    print_log(",\n");
    indent(idt+2); print_key("init_values"); print_log("[");
    bool is_head = true;
    for (int i=0; i < var->init_size; i++) {
      print_log(is_head ? "" : ",");
      is_head = false;
      print_log(" %ld", var->init_values[i]);
    }
    indent(idt+2); print_log(" ]");
  }
  print_log(",\n");
  indent(idt+2); print_key("type");
    print_ast_type(var->type, idt+2);

  print_log("\n");
  indent(idt); print_log("}");
}

static void print_ast_vars(Var *vars, int idt) {
  print_log("[");
  bool is_head = true;
  for (Var *var = vars; var; var=var->next) {
    print_log(is_head ? "\n" : ",\n");
    is_head = false;

    indent(idt+2); print_ast_var(var, idt+2);
  }
  print_log("\n");
  indent(idt); print_log("]");
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
  print_log("{ \n");

  indent(idt+2); print_key("struct");           print_log("\"Token\", \n");
  indent(idt+2); print_key("kind");             print_log("\"%s\", \n", token_kind_string(token->kind));

  indent(idt+2); print_key("string");
  if (token->kind == TK_STRING)
    print_log("\"\\%.*s\\\"\", \n", token->length -1, token->location);
  else
    print_log("\"%.*s\", \n", token->length, token->location);

  indent(idt+2); print_key("filename");         print_log("\"%s\", \n", token->filename);
  indent(idt+2); print_key("prev_is_space");    print_log(token->prev_is_space ? "true" : "false");

  print_log("\n");
  indent(idt);
  print_log("}");
}

static void print_ast_node(Node *node, int idt) {
  print_log("{\n");

  indent(idt+2); print_key("struct");     print_log("\"Node\",\n");
  indent(idt+2); print_key("kind");       print_log("\"%s\",\n", node_kind_string(node->kind));
  indent(idt+2); print_key("token");      print_ast_token(node->token, idt+2); print_log(",\n");
  indent(idt+2); print_key("value");      print_log("%ld", node->value);

  if (node->var_inits_size) {
    print_log(",\n");
    indent(idt+2); print_key("var_inits"); print_log("[");
    bool is_head = true;
    for (int i=0; i < node->var_inits_size; i++) {
      print_log(is_head ? "" : ",");
      is_head = false;
      print_log(" %ld", node->var_inits[i]);
    }
    print_log(" ]");
  }

  if (node->string) {
    print_log(",\n");
    indent(idt+2); print_key("string");       print_log("%.*s", node->token->length, node->token->location);
  }
  if (node->kind == ND_STMT_SWITCH) {
    print_log(",\n");
    indent(idt+2); print_key("have_default"); print_log(node->have_default ? "true" : "false");
  }

  if (node->kind == ND_LABEL_CASE) {
    print_log(",\n");
    indent(idt+2); print_key("case_num");       print_log("%d", node->case_num);
  }

  if (node->var) {
    print_log(",\n");
    indent(idt+2); print_key("var");
    print_ast_vars(node->var, idt+2);
  }
  if (node->member) {
    print_log(",\n");
    indent(idt+2); print_key("member");
    print_ast_member(node->member, idt+2);
  }
  if (node->type) {
    print_log(",\n");
    indent(idt+2); print_key("type");
    print_ast_type(node->type, idt+2);
  }

  if (node->func_name) {
    print_log(",\n");
    indent(idt+2); print_key("func_name");       print_log("\"%.*s\"", node->func_namelen, node->func_name);
  }
  if (node->func_args) {
    print_log(",\n");
    indent(idt+2); print_key("func_args");
    print_ast_nodes(node->func_args, idt+2);
  }

  if (node->left) {
    print_log(",\n");
    indent(idt+2); print_key("left");
    print_ast_node(node->left, idt+2);
  }
  if (node->right) {
    print_log(",\n");
    indent(idt+2); print_key("right");
    print_ast_node(node->right, idt+2);
  }
  if (node->cond) {
    print_log(",\n");
    indent(idt+2); print_key("cond");
    print_ast_node(node->cond, idt+2);
  }
  if (node->then) {
    print_log(",\n");
    indent(idt+2); print_key("then");
    print_ast_node(node->then, idt+2);
  }
  if (node->els) {
    print_log(",\n");
    indent(idt+2); print_key("els");
    print_ast_node(node->els, idt+2);
  }
  if (node->init) {
    print_log(",\n");
    indent(idt+2); print_key("init");
    print_ast_node(node->init, idt+2);
  }
  if (node->increment) {
    print_log(",\n");
    indent(idt+2); print_key("increment");
    print_ast_node(node->increment, idt+2);
  }
  if (node->body) {
    print_log(",\n");
    indent(idt+2); print_key("body");
    print_ast_nodes(node->body, idt+2);
  }
  if (node->cases) {
    print_log(",\n");
    indent(idt+2); print_key("cases");
    print_ast_nodes(node->cases, idt+2);
  }

  print_log("\n");
  indent(idt); print_log("}");
}

static void print_ast_nodes(Node *nodes, int idt) {
  print_log("[");
  bool is_head = true;
  for (Node *node = nodes; node; node=node->next) {
    print_log(is_head ? "\n" : ",\n");
    is_head = false;

    indent(idt+2); print_ast_node(node, idt+2);
  }
  print_log("\n");
  indent(idt); print_log("]");
}

static void print_ast_function(Function *func, int idt) {
  print_log("{\n");

  indent(idt+2); print_key("struct");     print_log("\"Function\",\n");
  indent(idt+2); print_key("name");       print_log("\"%.*s\",\n", func->namelen, func->name);
  indent(idt+2); print_key("var_byte");   print_log("%d,\n", func->var_byte);
  indent(idt+2); print_key("definition"); print_log(func->definition ? "true,\n" : "false,\n");
  indent(idt+2); print_key("argc");       print_log("%d", func->argc);
  if (func->argc) {
    print_log(",\n");
    indent(idt+2); print_key("args");
    print_ast_vars(func->args, idt+2);
  }
  print_log(",\n");
  indent(idt+2); print_key("type");
  print_ast_type(func->type, idt+2);

  if (func->node) {
    print_log(",\n");
    indent(idt+2); print_key("node");
    print_ast_node(func->node, idt+2);
  }

  print_log("\n");
  indent(idt); print_log("}");
}

static void print_ast_functions(Function *funcs, int idt) {
  print_log("[");
  bool is_head = true;
  for (Function *func = funcs; func; func = func->next) {
    print_log(is_head ? "\n" : ",\n");
    is_head = false;

    indent(idt+2); print_ast_function(func, idt+2);
  }
  print_log("\n");
  indent(idt); print_log("]");
}

void parse_log() {
  parse_logfile = fopen("ast.json","w");
  if (!parse_logfile)
    error("fail to open ast.json");

  print_ast_functions(functions, 0);

  fclose(parse_logfile);
}
