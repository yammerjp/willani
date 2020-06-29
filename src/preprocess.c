#include "willani.h"

Token head = {};

Token *preprocess(Token *token) {
  head.next = token;
  token = &head;

  while (token->next) {
    if (token->next->kind == TK_PREPROCESS_BEGIN) {
      Token *pre_begin = token;
      token = token->next;
      while(token && token->kind != TK_PREPROCESS_END)
        token = token->next;
      pre_begin->next = token->next;
      token = pre_begin;
    } else {
      token = token->next;
    }
  }
  return head.next;
}

