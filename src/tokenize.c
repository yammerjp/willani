#include "willani.h"

// ========== for debug ==========
static FILE *tokenize_log_file;

static char *token_kind_str(Token *token) {
  switch (token->kind) {
    case TK_RESERVED: return("TK_RESERVED");
    case TK_IDENT:    return("TK_INDENT  ");
    case TK_NUM:      return("TK_NUM     ");
    case TK_STRING:   return("TK_STRING  ");
    case TK_EOF:      return("TK_EOF     ");
    default : error("unexpected token->kind");
  }
}

static void tokenize_log(Token *token) {
  fprintf(tokenize_log_file, "%s, ", token_kind_str(token));
  fprintf(tokenize_log_file, "%.*s\n",token->length ,token->location);
}

char *get_line_head(Token *token) {
  // get line head
  char *head = token->location;
  while (user_input < head && head[-1] != '\n')
    head--;
  return head;
}

char *get_line_end(Token *token) {
  // get line end
  char *end = token->location;
  while (*end != '\n')
    end++;
  return end;
}

int get_line_number(char *line_head) {
  // get line number
  int line_num = 1;
  for (char *p  = user_input; p< line_head; p++)
    if (*p == '\n')
      line_num++;
  return line_num;
}


// ========== check token type ==========
bool is_identifer_token(Token *token) {
  return token->kind == TK_IDENT;
}

bool is_number_token(Token *token) {
  return token->kind == TK_NUM;
}

bool is_string_token(Token *token) {
  return token->kind == TK_STRING;
}

bool is_eof_token(Token *token) {
  return token->kind == TK_EOF;
}

bool equal(Token *token, char *str) {
  return strlen(str) == token->length &&
          !strncmp(token->location, str, token->length);
}


// ==========  read input and check token ==========
static int digit_token_length(char *p) {
  int i = 0;
  while (isdigit(p[i])) {
    i++;
  }
  return i;
}

static bool is_alnum(char c) {
  return ( 'a' <= c && c <= 'z' ) ||
         ( 'A' <= c && c <= 'Z' ) ||
         ( '0' <= c && c <= '9' ) ||
         ( c == '_' ) ;
}

static bool is_all_alnum(char *c) {
  for (int i=0; i < strlen(c); i++){
    if (!is_alnum(c[i])) {
      return false;
    }
  }
  return true;
}

static int identifer_token_length(char *p) {
  if (!isalpha(*p)) {
    return 0;
  }
  int i = 1;
  while (is_alnum(p[i])) {
    i++;
  }
  return i;
}

static int reserved_token_length(char *p) {
  char tokens[][7] = { "return", "sizeof", "while", "else", "long", "char", "int", "for", "if", "==", "!=", "<=", ">=", "+", "-", "*", "/", "(", ")", ">", "<", ";", "=", "{", "}", ",", "&", "[", "]"};
  for (int i=0; i<sizeof(tokens); i++) {
    char *str = tokens[i];
    int len = strlen(str);
    if ( strncmp(p, str, len) == 0
      && ( !is_all_alnum(str) || !is_alnum(p[len]) ) // not include 'ifn' and so on
    ) {
      return len;
    }
  }
  return 0;
}

static int string_token_length(char *p) {
  if (*p != '"') {
    return 0;
  }
  for (int i = 1; p[i]; i++) {
    if (p[i] == '"') {
      return i+1;
    }
  }
  error("unclosed \"");
}

// ========== new token ==========
// Create new (tail) token, Connect to the current token and Return new (tail) token.
static Token *new_token(TokenKind kind, Token *current, char *location, int length) {
  Token *token = calloc(1, sizeof(Token));
  token->kind = kind;
  token->location = location;
  token->length = length;
  current->next = token;

  // for debug
  tokenize_log(token);

  return token;
}


// ========== tokenize ==========
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

    int dlen = digit_token_length(p);
    if (dlen > 0) {
      current = new_token(TK_NUM, current, p, dlen);
      p += dlen;
      continue;
    }

    int rlen = reserved_token_length(p);
    if (rlen > 0) {
      current = new_token(TK_RESERVED, current, p, rlen);
      p += rlen;
      continue;
    }

    int ilen = identifer_token_length(p);
    if (ilen > 0) {
      current = new_token(TK_IDENT, current, p, ilen);
      p += ilen;
      continue;
    }

    int slen = string_token_length(p);
    if (slen > 0) {
      current = new_token(TK_STRING, current, p, slen);
      p += slen;
      continue;
    }

    error_at_token(current, "Invalid token");
  }

  new_token(TK_EOF, current, p, 0);

  // for debug
  fclose(tokenize_log_file);

  return head.next;
}
