#include "willani.h"

Token head = {};
static bool is_same(Token *t1, Token *t2);

typedef struct DefineDirective DefineDirective;
struct DefineDirective {
  Token *ident;
  Token *params;
  Token *replacings;
  Token *preprocess_end;
  DefineDirective *next;
};

DefineDirective *defines;

static void add_defines(Token *ident, Token *params, Token *replacings, Token *preprocess_end) {
  DefineDirective *new = calloc(1, sizeof(DefineDirective));
  new->ident = ident;
  new->params = params;
  new->replacings = replacings;
  new->preprocess_end = preprocess_end;

  new->next = defines;
  defines = new;
}

static bool find_defines(Token *ident) {
  for (DefineDirective *define = defines; define; define = define->next) {
    if (ident->length == define->ident->length && !strncmp(ident->location, define->ident->location, ident->length))
      return true;
  }
  return false;
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
  copied->file = token->file;
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

static void define_preprocess_line(Token **rest, Token *token) {
  // Read preprocess line
  //token->next->kind is TK_PREPROCESS_BEGIN
  Token *prebegin = token;
  token = token->next->next;

  // define
  if (!equal(token, "define"))
    error_at(token, "expected define (preprocess directive)");
  token = token->next;

  // ident
  if (!is_identifer_token(token))
    error_at(token, "expected identifer (preprocess directive)");
  Token *ident = token;
  token = token->next;

  // params
  Token preparams = {};
  Token *params_tail = &preparams;
  if (equal(token, "(") && !token->prev_is_space) {
    // "(" (param ("," param)*)? ")"
    token = token->next;
    while (!equal(token, ")")) {
      if (!is_identifer_token(token))
        error_at(token, "expected identifer of define macro's param (preprocess directive)");
      params_tail = params_tail->next = copy_tail(token);
      token = token->next;
      if (equal(token, ")"))
        break;
      if (!equal(token, ","))
        error_at(token, "expected , of define macro's param (preprocess directive)");
      token = token->next;
    }
    token = token->next;
    params_tail->next = NULL;
  }

  // replacements
  Token *replacings = token;

  // find the end of preprocess line
  while (token->kind != TK_PREPROCESS_END)
    token = token->next;
  Token *end = token;

  add_defines(ident, preparams.next, replacings, end);

  // delete preprocess line tokens from tokens row
  prebegin->next = end->next;
  *rest = prebegin;
}

static void define_replace(Token *pre_begin) {
  // Replace tokens
  Token *token = pre_begin;
  while (token && token->next) {
    if (token->next->kind == TK_PREPROCESS_BEGIN) {
      token = token->next;
      while (token->kind != TK_PREPROCESS_END)
        token = token->next;
      continue;
    }
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
      token->next = copies(defines->replacings, defines->preprocess_end, dest);
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
      error_at(token->next, "expected ( of macro's parameter (define directive preprocess)");
    token = token->next->next;

    Token specified_params[100] = {};
    Token *specified_params_tail = specified_params;
    int parens_ct = 0;
    int num = 0;
    while (!equal(token, ")") || parens_ct) {
      if (!parens_ct && equal(token, ",")) {
        specified_params_tail = specified_params + ++num;
      } else {
        if (equal(token, "("))
          parens_ct++;
        else if (equal(token, ")"))
          parens_ct--;
        specified_params_tail = specified_params_tail->next = copy_tail(token);
      }
      token = token->next;
    }
    token = token->next;

    // replace identifer and parameters to replacement of define line
    //  ident(1, 2, 3) => C+B+A     (e.x.)
    dest = token;
    prereplaced->next = copies(defines->replacings, defines->preprocess_end, dest);
    token = prereplaced->next;
    while (token->next != dest)
      token = token->next;

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
      Token *tk_dest = tk->next->next;
      tk->next = (specified_params[num-1]).next;
      while (tk->next)
        tk = tk->next;
      tk->next = tk_dest;
    }
  }
}

static char *including_filepath(Token *str_token) {
  char *included_filepath = str_token->file->path;
  char *new_filename = str_token->location+1;
  int new_filenamelen = str_token->length-2;

  char *dir_name = included_filepath;
  int dir_namelen = strlen(included_filepath);
  for (;dir_namelen > 0; dir_namelen--) {
    // not yet supported escaping slash
    if (dir_name[dir_namelen-1] == '/')
      break;
  }

  char *new_filepath = calloc(dir_namelen + new_filenamelen + 1, sizeof(char));
  memcpy(new_filepath, dir_name, dir_namelen);
  memcpy(new_filepath + dir_namelen, new_filename, new_filenamelen);
  new_filepath[dir_namelen + new_filenamelen] = '\0';

  return new_filepath;
}

static void include_preprocess_line(Token **rest, Token *token) {
  // token->next->kind is TK_PREPROCESS_BEGIN
  Token *prebegin = token;
  token = token->next->next;

  if (!equal(token, "include"))
    error_at(token, "expected include (preprocess directive)");
  token = token->next;

  if (!is_string_token(token))
    error_at(token, "not yet supported include standard library");
  char *filepath = including_filepath(token);
  token = token->next;

  if (token->kind != TK_PREPROCESS_END)
    error_at(token, "expected preprocess line end");
  Token *end = token;

  SourceFile *sf = read_file(filepath);

  // replace from include preprocess line to including file text
  prebegin->next = tokenize(sf);

  token = prebegin;
  while (token->next->kind != TK_EOF)
    token = token->next;
  token->next = end->next;

  *rest = prebegin;
}

static void ifdef_ifndef_preprocess_line(Token **rest, Token *token) {
  // token->next->kind is TK_PREPROCESS_BEGIN
  Token *prebegin = token;
  token = token->next->next;

  bool invert_cond;
  if (equal(token, "ifdef"))
    invert_cond = false;
  else if (equal(token, "ifndef"))
    invert_cond = true;
  else
    error_at(token, "expected ifndef (preprocess directive)");
  token = token->next;

  if (!is_identifer_token(token))
    error_at(token, "expected identifer of ifndef preprocess line");
  bool condition = find_defines(token);
  if (invert_cond)
    condition = !condition;
  token = token->next;

  if (token->kind != TK_PREPROCESS_END)
    error_at(token, "expected preprocess line end");
  Token *end = token;

  // delete preprocess line tokens from tokens row
  prebegin->next = end->next;

  Token *else_prebegin = NULL;
  Token *else_end = NULL;
  for (int if_depth=0;;token = token->next) {
    if (!token || !token->next)
      error("need #endif of preprocess line");
    if (token->next->kind != TK_PREPROCESS_BEGIN)
      continue;
    if (equal(token->next->next, "ifndef")) {
      if_depth++;
      continue;
    }
    if (equal(token->next->next, "else")) {
      else_prebegin = token;
      token = token->next->next;
      if (token->next->kind != TK_PREPROCESS_END)
        error_at(token->next, "expected preprocess line end");
      else_end = token->next;

      // delete preprocess line tokens from tokens row
      else_prebegin->next = else_end->next;
      continue;
    }
    if (equal(token->next->next, "endif")) {
      if (if_depth-- > 0)
        continue;
      break;
    }
    if (!token->next->next || !token->next->next->next)
      error("need #endif of preprocess line");
  }
  Token *endif_prebegin = token;
  token = token->next->next->next;

  if (token->kind != TK_PREPROCESS_END)
    error_at(token, "expected preprocess line end");
  Token *endif_end = token;
  token = token->next;

  // delete preprocess line tokens from tokens row
  endif_prebegin->next = endif_end->next;

  if (else_prebegin) {
    if (condition)
      else_prebegin->next = endif_end->next;
    else
      prebegin->next = else_end->next;
  } else if (!condition) {
    // delete ifndef ~ endif
    prebegin->next = endif_end->next;
  }

  *rest = prebegin;
}

Token *preprocess(Token *token) {
  head.next = token;
  token = &head;

  while (token->next) {
    if (token->next->kind == TK_PREPROCESS_BEGIN) {
      if (!token->next->next)
        error_at(token->next, "faild to tokenize. expected TK_PREPROCESS_BEGIN is pair with TK_PREPROCESS_END");
      if (equal(token->next->next, "define")) {
        define_preprocess_line(&token, token);
        define_replace(token);
        continue;
      }
      if (equal(token->next->next, "include")) {
        include_preprocess_line(&token, token);
        continue;
      }
      if (equal(token->next->next, "ifndef")
       || equal(token->next->next, "ifdef")) {
        ifdef_ifndef_preprocess_line(&token, token);
        continue;
      }
      error_at(token->next->next, "unknown preprocess directive");

    }
    token = token->next;
  }
  return head.next;
}

