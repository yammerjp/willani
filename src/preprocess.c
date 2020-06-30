#include "willani.h"

Token head = {};

static Token *copy(Token *token) {
  Token *copied = calloc(1, sizeof(Token));
  copied->location = token->location;
  copied->length = token->length;
  copied->kind = token->kind;
  copied->next = token->next;
  return copied;
}

static void define(Token **rest, Token *pre_begin) {
  // Read preprocess line
  Token *token = pre_begin->next; //token->kind is TK_PREPROCESS_BEGIN
  token = token->next;

  if (!equal(token, "define"))
    error_at(token->location, "expected define (preprocess directive)");
  token = token->next;

  Token *ident = token;
  token = token->next;

  Token *replacement = token;
  token = token->next;

  if (token->kind != TK_PREPROCESS_END)
    error_at(token->location, "expected end of preprocess line");

  pre_begin->next = token->next;
  *rest = pre_begin;

  // Replace token
  token = pre_begin;
  while (token && token->next) {
    if (ident->kind == token->next->kind
     && ident->length == token->next->length
     && !strncmp(ident->location, token->next->location, ident->length)
    ) {
      Token *dest = token->next->next;
      Token *copied = copy(replacement);
      token->next = copied;
      copied->next = dest;
    }
    token = token->next;
  }
}

Token *preprocess(Token *token) {
  head.next = token;
  token = &head;

  while (token->next) {
    if (token->next->kind == TK_PREPROCESS_BEGIN) {
      if (!token->next->next)
        error_at(token->next->location, "faild to tokenize. expected TK_PREPROCESS_BEGIN is pair with TK_PREPROCESS_END");
      if (equal(token->next->next, "define")) {
        define(&token, token);
        continue;
      }
      error_at(token->next->next->location, "unknown preprocess directive");
    }
    token = token->next;
  }
  return head.next;
}

