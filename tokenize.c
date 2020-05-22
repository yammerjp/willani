#include "c2.h"

// for debug
static FILE *tokenize_log_file;

bool is_number_token(Token *token) {
  return token->kind == TK_NUM;
}

bool equal(Token *token, char *str) {
  return strlen(str) == token->length &&
          !strncmp(token->location, str, token->length);
}

long get_number(Token *token) {
  if (token->kind != TK_NUM)
    error_at(token, "expected a number");
  return token->value;
}

// Create new (tail) token, Connect to the current token and Return new (tail) token.
static Token *new_token(TokenKind kind, Token *current, char *location, int length, long value) {
  Token *newtoken = calloc(1, sizeof(Token));
  newtoken->kind = kind;
  newtoken->location = location;
  newtoken->length = length;
  newtoken->value = value;
  current->next = newtoken;

  // for debug
  fprintf(tokenize_log_file, "%.*s\n",newtoken->length ,newtoken->location);

  return newtoken;
}

// To return 0 is not reserved token
static int reserved_token_length(char *p) {
  char tokens[][3] = { "==", "!=", "<=", ">=", "+", "-", "*", "/", "(", ")", ">", "<", };
  for (int i=0; i<sizeof(tokens); i++) {
    if ( strncmp(p, tokens[i], strlen(tokens[i])) == 0 )
      return strlen(tokens[i]);
  }
  return 0;
}

Token *tokenize(char *p) {
  Token head = {};
  Token *current = &head;

  // for debug
  tokenize_log_file = fopen("tokenize.log","w");
  if (tokenize_log_file == NULL) {
    error("fail to open tokenize.log");
  }

  while (*p) {
    if (isspace(*p)) {
      p++;
      continue;
    }

    if (isdigit(*p)) {
      char *p_old = p;
      long value = strtol(p, &p, 10); // with update p
      current = new_token(TK_NUM, current, p_old, p - p_old, value);
      continue;
    }

    int length = reserved_token_length(p);
    if (length == 0) {
      error_at(current, "Invalid token");
    }
    current = new_token(TK_RESERVED, current, p ,length, 0); //with update p
    p += length;
    continue;
  }

  new_token(TK_EOF, current, p, 0, 0);

  // for debug
  fclose(tokenize_log_file);

  return head.next;
}
