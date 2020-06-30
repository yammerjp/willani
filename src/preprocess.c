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

static Token *copies(Token *src, Token *uncopies, Token *tail_next) {
  Token predest_head = {};
  Token *predest = &predest_head;

  while (src != uncopies) {
    predest->next = copy(src);
    src = src->next;
    predest = predest->next;
  }
  predest->next = tail_next;
  return predest_head.next;
}

static bool is_same(Token *t1, Token *t2) {
  return (
       t1->kind == t2->kind
    && t1->length == t2->length
    && !strncmp(t1->location, t2->location, t1->length)
  );
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

  Token *replacings = token;
  token = token->next;

  while (token->kind != TK_PREPROCESS_END)
    token = token->next;
  Token *unreplacings = token;

  pre_begin->next = token->next;
  *rest = pre_begin;

  // Replace tokens
  token = pre_begin;
  while (token && token->next) {
    if (is_same(ident, token->next)) {
      Token *dest = token->next->next;
      token->next = copies(replacings, unreplacings, dest);
      while (token->next !=dest)
        token = token->next;
      continue;
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

