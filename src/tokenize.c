#include "willani.h"

// ========== for debug ==========
static FILE *tokenize_log_file;

static char *token_kind_str(Token *token) {
  switch (token->kind) {
    case TK_RESERVED:         return("TK_RESERVED        ");
    case TK_IDENT:            return("TK_IDENT           ");
    case TK_NUM:              return("TK_NUM             ");
    case TK_STRING:           return("TK_STRING          ");
    case TK_CHAR:             return("TK_CHAR            ");
    case TK_EOF:              return("TK_EOF             ");
    case TK_PREPROCESS_BEGIN: return("TK_PREPROCESS_BEGIN");
    case TK_PREPROCESS_END:   return("TK_PREPROCESS_END  ");
    default : error("unexpected token->kind");
  }
}

static void tokenize_log(Token *token) {
  fprintf(tokenize_log_file, "%s, ", token_kind_str(token));
  fprintf(tokenize_log_file, "%.*s\n",token->length ,token->location);
}

char *get_line_head(char *head) {
  // get line head
  while (user_input < head && head[-1] != '\n')
    head--;
  return head;
}

char *get_line_end(char *end) {
  while (*end != '\n')
    end++;
  return end;
}

int get_line_number(char *line_head) {
  // get line number
  int line_num = 1;
  for (char *p  = user_input; p < line_head; p++)
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

bool is_char_token(Token *token) {
  return token->kind == TK_CHAR;
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
  bool base_is_16 = false;
  if (p[0] == '0') {
    i++;
    switch (p[1]) {
      case 'x':
      case 'X':
        base_is_16 = true;
      case 'b':
      case 'B':
        i++;
    }
  }
  while ('0'<=p[i]&&p[i]<='9' || base_is_16 && ('a'<=p[i]&&p[i]<='f' || 'A'<=p[i]&&p[i]<='F'))
    i++;
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
    if (!is_alnum(c[i]))
      return false;
  }
  return true;
}

static int identifer_token_length(char *p) {
  if (!isalpha(*p) && *p != '_')
    return 0;

  int i = 1;
  while (is_alnum(p[i]))
    i++;
  return i;
}

static int reserved_token_length(char *p) {
  char reserved_words[][8] = {
    "continue",
    "default",
    "typedef",
    "struct",
    "return",
    "sizeof",
    "static",
    "switch",
    "break",
    "_Bool",
    "while",
    "case",
    "else",
    "long",
    "char",
    "enum",
    "int",
    "for",
    "if",
    "==",
    "!=",
    "<=",
    ">=",
    "->",
    "+=",
    "-=",
    "*=",
    "/=",
    "%=",
    "++",
    "--",
    "||",
    "&&",
    "+",
    "-",
    "*",
    "/",
    "(",
    ")",
    ">",
    "<",
    ";",
    ":",
    "=",
    "{",
    "}",
    ",",
    "&",
    "[",
    "]",
    ".",
    "%",
    "!",
    "~",
    "?",
    "|",
    "^",
  };
  int words_length = sizeof(reserved_words)/sizeof(*reserved_words);
  for (int i=0; i < words_length; i++) {
    char *str = reserved_words[i];
    int len = strlen(str);
    if ( strncmp(p, str, len) != 0 )
      continue;
    if ( is_all_alnum(str) && is_alnum(p[len]) ) // not include 'ifn' and so on
      continue;
    return len;
  }
  return 0;
}

static int char_token_length(char *p) {
  if (p[0] != '\'')
    return 0;

  int length =  (p[1] == '\\') ? 4 : 3;
  if (p[length-1] !='\'')
    error("quote is invalid");
  return length;
}

static int string_token_length(char *p) {
  if (*p != '"')
    return 0;

  for (int i = 1; p[i] && p[i] != '\n'; i++) {
    if (p[i] == '\\') {
      // Skip next charactor
      i++;
      continue;
    }

    if (p[i] == '"')
      return i+1;
  }
  error("unclosed \"");
}

// ========== new token ==========
// Create new (tail) token, Connect to the current token and Return new (tail) token.
static Token *new_token(TokenKind kind, Token *current, char *location, int length, bool prev_is_space) {
  Token *token = calloc(1, sizeof(Token));
  token->kind = kind;
  token->location = location;
  token->length = length;
  token->prev_is_space = prev_is_space;
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
  if (tokenize_log_file == NULL)
    error("fail to open tokenize.log");

  bool is_line_head = true;
  bool is_preprocess_line = false;
  bool prev_is_space = false;

  while (*p) {
    // Begin preprocess
    if (*p == '#' && is_line_head) {
      current = new_token(TK_PREPROCESS_BEGIN, current, p, 1, prev_is_space);
      is_preprocess_line = true;
      p++;
      continue;
    }
    // Finish preprocess
    if (*p == '\n') {
      if (is_preprocess_line)
        current = new_token(TK_PREPROCESS_END, current, p, 1, prev_is_space);
      p++;
      is_preprocess_line = false;
      is_line_head = true;
      prev_is_space = true;
      continue;
    }
    // Skip space
    if (isspace(*p)) {
      p++;
      prev_is_space = true;
      continue;
    }
    is_line_head = false;

    // Skip comment
    if (!strncmp(p, "//", 2)) {
      p +=2;
      while (*p != '\n')
        p++;
      prev_is_space = true;
      continue;
    }
    if (!strncmp(p, "/*", 2)) {
      char *q =  strstr(p+2, "*/");
      if (!q)
        error("comment is not closed");
      p = q+2;
      prev_is_space = true;
      continue;
    }

    // number
    int dlen = digit_token_length(p);
    if (dlen > 0) {
      current = new_token(TK_NUM, current, p, dlen, prev_is_space);
      p += dlen;
      prev_is_space = false;
      continue;
    }

    // reserved word
    int rlen = reserved_token_length(p);
    if (rlen > 0) {
      current = new_token(TK_RESERVED, current, p, rlen, prev_is_space);
      p += rlen;
      prev_is_space = false;
      continue;
    }

    // identifer
    int ilen = identifer_token_length(p);
    if (ilen > 0) {
      current = new_token(TK_IDENT, current, p, ilen, prev_is_space);
      p += ilen;
      prev_is_space = false;
      continue;
    }

    // ' ... '
    int clen = char_token_length(p);
    if (clen > 0) {
      current = new_token(TK_CHAR, current, p, clen, prev_is_space);
      p += clen;
      prev_is_space = false;
      continue;
    }

    // " ... "
    int slen = string_token_length(p);
    if (slen > 0) {
      current = new_token(TK_STRING, current, p, slen, prev_is_space);
      p += slen;
      prev_is_space = false;
      continue;
    }

    error_at(current, "Invalid token");
  }

  new_token(TK_EOF, current, p, 0, prev_is_space);

  // for debug
  fclose(tokenize_log_file);

  return head.next;
}
