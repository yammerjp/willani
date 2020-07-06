#include "willani.h"

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


void tokenize_log_open() {
  tokenize_log_file = fopen("tokenize.log","w");
  if (tokenize_log_file == NULL)
    error("fail to open tokenize.log");
}

void tokenize_log_close() {
  fclose(tokenize_log_file);
}

void tokenize_log(Token *token) {
  fprintf(tokenize_log_file, "%s, ", token_kind_str(token));
  fprintf(tokenize_log_file, "%.*s\n",token->length ,token->location);
}


