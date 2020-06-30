#include "willani.h"

Token head = {};
static bool is_same(Token *t1, Token *t2);

typedef struct DefineDirective DefineDirective;
struct DefineDirective {
  Token *ident;
  Token *params;
  Token *replacings;
  Token *unreplacings;
  DefineDirective *next;
};

DefineDirective *defines;

static void add_defines(Token *ident, Token *params, Token *replacings, Token *unreplacings) {
  DefineDirective *new = calloc(1, sizeof(DefineDirective));
  new->ident = ident;
  new->params = params;
  new->replacings = replacings;
  new->unreplacings = unreplacings;

  new->next = defines;
  defines = new;
}

static int param_num(Token *param_ident) {
  int num = 1;
  for (Token *tk = defines->params; tk; tk = tk->next) {
    if (is_same(tk, param_ident))
      return num;
    num++;
  }
  return 0;
}
static Token *param(int num) {
  int ct = 1;
  Token *param_ident;
  for (Token *tk = defines->params; tk; tk = tk->next) {
    if (ct == num) {
      param_ident = tk;
      break;
    }
    ct++;
  }
  error("param is not found");
}

static Token *copy(Token *token) {
  Token *copied = calloc(1, sizeof(Token));
  copied->location = token->location;
  copied->length = token->length;
  copied->kind = token->kind;
  copied->next = token->next;
  return copied;
}

static Token *copy_tail(Token *token) {
  Token *copied = copy(token);
  copied->next = NULL;
  return copied;
}

static Token *copies(Token *src, Token *uncopies, Token *tail_next) {
  Token predest_head = {};
  Token *predest = &predest_head;

  while (src != uncopies) {
    predest = predest->next = copy(src);
    src = src->next;
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

static void define_preprocess_line(Token **rest, Token *pre_begin) {
  // Read preprocess line
  Token *token = pre_begin->next; //token->kind is TK_PREPROCESS_BEGIN
  token = token->next;

  // define
  if (!equal(token, "define"))
    error_at(token->location, "expected define (preprocess directive)");
  token = token->next;

  // ident
  Token *ident = token;
  token = token->next;


  Token preparams = {};
  Token *params_tail = &preparams;
  if (equal(token, "(") && !token->prev_is_space) {
    // "(" (param ("," param)*)? ")"
    token = token->next;
    while (!equal(token, ")")) {
      if (!is_identifer_token(token))
        error_at(token->location, "expected identifer of define macro's param (preprocess directive)");
      params_tail = params_tail->next = copy_tail(token);
      token = token->next;
      if (equal(token, ")"))
        break;
      if (!equal(token, ","))
        error_at(token->location, "expected , of define macro's param (preprocess directive)");
      token = token->next;
    }
    token = token->next;
    params_tail->next = NULL;
  }

  // replacements
  Token *replacings = token;
  token = token->next;

  while (token->kind != TK_PREPROCESS_END)
    token = token->next;
  Token *unreplacings = token;

  // delete preprocess line tokens from tokens row
  pre_begin->next = token->next;
  *rest = pre_begin;

  add_defines(ident, preparams.next, replacings, unreplacings);

  // debug print
  fprintf(stderr, "defines:\n");
  fprintf(stderr, " ident: %.*s\n", defines->ident->length, defines->ident->location);
  for (Token *tk = defines->params; tk; tk = tk->next)
    fprintf(stderr, " param: %.*s\n", tk->length, tk->location);
  for (Token *tk = defines->replacings; tk && tk != defines->unreplacings; tk = tk->next)
    fprintf(stderr, " replace: %.*s\n", tk->length, tk->location);
}

static void define_replace(Token *pre_begin){
  // Replace tokens
  Token *token = pre_begin;
  while (token && token->next) {
    if (!is_same(defines->ident, token->next)) {
      // not replacing
      token = token->next;
      continue;
    }
    // replacing

    // ident
    Token *prereplaced = token;
    Token *dest;
    if (!defines->params) {
      // ident => replacement
      dest = token->next->next;
      token->next = copies(defines->replacings, defines->unreplacings, dest);
      while (token->next !=dest)
        token = token->next;
      continue;
    }

    // expand parameters

    // #define ident(A,B,C) C+B+A     (e.x.)

    // read specified parameters and register to array
    // (1, 2, 3)      (e.x.)
    token = token->next;
    if (!equal(token->next, "("))
      error_at(token->next->location, "expected ( of macro's parameter (define directive preprocess)");
    token = token->next->next;

    Token specified_params[100] = {};
    Token *specified_params_tail = &(specified_params[0]);
    int parens_ct = 0;
    int num = 0;
    while (!equal(token, ")") || parens_ct) {
      if (!parens_ct && equal(token, ",")) {
        specified_params_tail = &(specified_params[++num]);
      } else {
        if (equal(token, "("))
          parens_ct++;
        else if (equal(token, ")"))
          parens_ct--;
        specified_params_tail = specified_params_tail->next = copy_tail(token);
      }
      token = token->next;
    }
    // debug
    for(int i = 0; (specified_params[i]).next; i++)
      for (Token *tk = (specified_params[i]).next; tk; tk = tk->next)
        fprintf(stderr, " specified param[%d]: %.*s\n", i, tk->length, tk->location);

    token = token->next;


    // replace identifer and parameters to replacement of define line
    //  ident(1, 2, 3) => C+B+A     (e.x.)
    dest = token;
    prereplaced->next = copies(defines->replacings, defines->unreplacings, dest);
    token = prereplaced->next;
    while (token->next != dest)
      token = token->next;

    // debug
    fprintf(stderr, "expand macro parameters\n");

    // replace parameters of define line from specified parameters array
    // C+B+A => 3+2+1              (e.x.)
    Token *tk = prereplaced;
    while (tk->next != dest) {
      int num = param_num(tk->next);
      if (num==0) {
        // not param
        tk = tk->next;
        continue;
      }
      // replace param to the value specified on macro
      // debug
      fprintf(stderr, "replace token: %.*s (num: %d)\n", tk->next->length, tk->next->location, num);
      Token *tk_dest = tk->next->next;
      tk->next = (specified_params[num-1]).next;
      while (tk->next)
        tk = tk->next;
      tk->next = tk_dest;
    }
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
        define_preprocess_line(&token, token);
        define_replace(token);
        continue;
      }
      error_at(token->next->next->location, "unknown preprocess directive");
    }
    token = token->next;
  }
  return head.next;
}

