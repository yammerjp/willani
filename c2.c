#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// tokenize
typedef enum {
  TK_RESERVED, // Keywords or punctuators
  TK_NUM, // Numeric literals
  TK_EOF, // End-of-file markers
} TokenKind;

typedef struct Token Token;

struct Token {
  TokenKind kind;
  Token *next;
  long value; // If kind is TK_NUM, its value.
  char *location;
  int length;
};

Token *token;

char *user_input;

static void error_at(Token *prevtoken, char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);

  int position = (prevtoken->location + prevtoken->length) - user_input;
  fprintf(stderr, "%s\n", user_input);
  fprintf(stderr, "%*s", position, "");
  fprintf(stderr, "^ ");

  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

static void error(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

static bool is_number_token(Token *token) {
  return token->kind == TK_NUM;
}

static bool equal(Token *token, char *str) {
  return strlen(str) == token->length &&
          !strncmp(token->location, str, token->length);
}

static long get_number(Token *token) {
  if (token->kind != TK_NUM)
    error_at(token, "expected a number");
  return token->value;
}

// Create new (tail) token, Connect to the current token and Return new (tail) token.
static Token *new_token(TokenKind kind, Token *current, char *location, int length) {
  Token *newtoken = calloc(1, sizeof(Token));
  newtoken->kind = kind;
  newtoken->location = location;
  newtoken->length = length;
  current->next = newtoken;
  return newtoken;
}

// To return 0 is not reserved token
int reserved_token_length(char *p) {
  char tokens[][2] = { "+", "-", "*", "/", "(", ")" };
  for (int i=0; i<sizeof(tokens); i++) {
    if ( strncmp(p, tokens[i], strlen(tokens[i])) == 0 )
      return strlen(tokens[i]);
  }
  return 0;
}

Token *tokenize(char *p) {
  Token head = {};
  Token *current = &head;

  while (*p) {
    if (isspace(*p)) {
      p++;
      continue;
    }

    if (isdigit(*p)) {
      current = new_token(TK_NUM, current, p, 0); // length is still unknown
      char *p_old = p;
      current->value = strtol(p, &p, 10); // with update p
      current->length = p - p_old;
      continue;
    }

    int length = reserved_token_length(p);
    if (length == 0) {
      error_at(current, "Invalid token");
    }
    current = new_token(TK_RESERVED, current, p ,length); //with update p
    p += length;
    continue;
  }

  new_token(TK_EOF, current, p, 0);
  return head.next;
}

void print_token(FILE *logfile, Token *token) {
  for(;;) {
    if (token->kind == TK_EOF) {
      return;
    }
    fprintf(logfile, "%.*s\n",token->length ,token->location);
    token = token->next;
  }
}

void tokenize_log(Token* head) {
  FILE *logfile;
  logfile = fopen("tokenize.log","w");
  if (logfile == NULL) {
    error("fail to open tokenize.log");
  }
  Token *token = head;
  print_token(logfile, token);

  fclose(logfile);
}

// parse
typedef enum {
  ND_ADD,
  ND_SUB,
  ND_MUL,
  ND_DIV,
  ND_NUM,
} NodeKind;

typedef struct Node Node;

struct Node {
  NodeKind kind;
  Node *left;
  Node *right;
  long value;
};

Node *new_node_op2(NodeKind kind, Node *left, Node *right) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = kind;
  node->left = left;
  node->right = right;
  return node;
}

Node *new_node_num(long value) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_NUM;
  node->value = value;
  return node;
}

Node *primary(Token **rest, Token *token);
Node *mul(Token **rest, Token *token);
Node *unary(Token **rest, Token *token);

// expr = mul ("+" mul | "-" mul)*
Node *expr(Token **rest, Token *token) {
  Node *node = mul(&token, token);

  for(;;) {
    if (equal(token, "+")){
      token = token->next;
      node = new_node_op2(ND_ADD, node, mul(&token, token));
    } else if (equal(token, "-")) {
      token = token->next;
      node = new_node_op2(ND_SUB, node, mul(&token, token));
    } else {
      *rest = token;
      return node;
    }
  }
}

// mul = unary ("*" unary | "/" unary)*
Node *mul(Token **rest, Token *token) {
  Node *node = unary(&token, token);

  for(;;) {
    if (equal(token, "*")) {
      token = token->next;
      node = new_node_op2(ND_MUL, node, unary(&token, token));
    } else if (equal(token, "/")) {
      token = token->next;
      node = new_node_op2(ND_DIV, node, unary(&token, token));
    } else {
      *rest = token;
      return node;
    }
  }
}

// unary   = ("+" | "-")? primary
Node *unary(Token **rest, Token *token) {
  if (equal(token,"+")) {
    token = token->next;
    Node *node = primary(&token, token);
    *rest = token;
    return node;
  }
  if (equal(token,"-")) {
    token = token->next;
    Node *node = new_node_op2(ND_SUB, new_node_num(0), primary(&token, token));
    *rest = token;
    return node;
  }
  Node *node = primary(&token, token);
  *rest = token;
  return node;
}

// primary = num | "(" expr ")"
Node *primary(Token **rest, Token *token) {
  if (is_number_token(token)) {
    Node *node = new_node_num(get_number(token));
    token = token->next;
    *rest = token;
    return node;
  }

  if (!equal(token,"(")) {
    error_at(token, "expected (");
  }

  token = token->next;
  Node *node = expr(&token, token);

  if (!equal(token,")")) {
    error_at(token, "expected )");
  }
  token = token->next;
  *rest = token;
  return node;
}

void code_generate(Node *node) {
  if (node->kind == ND_NUM) {
    printf("  push %ld\n", node->value);
    return;
  }

  // expect op2
  code_generate(node->left);
  code_generate(node->right);

  printf("  pop rdi\n");
  printf("  pop rax\n");

  switch (node->kind) {
  case ND_ADD:
    printf("  add rax, rdi\n");
    break;
  case ND_SUB:
    printf("  sub rax, rdi\n");
    break;
  case ND_MUL:
    printf("  imul rax, rdi\n");
    break;
  case ND_DIV:
    printf("  cqo\n");
    printf("  idiv rdi\n");
    break;
  }
  printf("  push rax\n");
}

void print_node(FILE *logfile, Node *node, int depth) {
  fprintf(logfile, "%*s",depth*2, "");

  if (node->kind == ND_NUM) {
    fprintf(logfile, "%ld\n",node->value);
  }
  if (node->kind == ND_ADD) {
    fprintf(logfile, "+\n");
  }
  if (node->kind == ND_SUB) {
    fprintf(logfile, "-\n");
  }
  if (node->kind == ND_MUL) {
    fprintf(logfile, "*\n");
  }
  if (node->kind == ND_DIV) {
    fprintf(logfile, "/\n");
  }
}

void print_nodes(FILE *logfile, Node *node, int depth) {
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

int main(int argc, char **argv) {
  if (argc != 2) {
    error("%s: invalid number of arguments.\n", argv[0]);
  }

  user_input = argv[1];
  token = tokenize(user_input);

  tokenize_log(token);

  // parse
  Node *node = expr(&token, token);

  parse_log(node);

  printf(".intel_syntax noprefix\n");
  printf(".globl main\n");
  printf("main:\n");

  code_generate(node);

  printf("  pop rax\n");
  printf("  ret\n");
  return 0;
}
