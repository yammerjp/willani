#include "willani.h"

void error_at(char *loc, char *msg) {
  // get line head
  char *line = loc;
  while (user_input < line && line[-1] != '\n')
    line--;

  // get line end
  char *end = loc;
  while (*end != '\n')
    end++;

  // get line number
  int line_num = 1;
  for (char *p  = user_input; p< line; p++)
    if (*p == '\n')
      line_num++;

  // print filename and line number
  int indent = fprintf(stderr, "%s:%d: ",filename, line_num);
  fprintf(stderr, "%.*s\n", (int)(end-line), line);

  // print error position
  int pos = loc - line + indent;
  fprintf(stderr, "%*s", pos, "");
  fprintf(stderr, "^ %s\n", msg);
  exit(1);
}

void error_at_token(Token *token, char *msg) {
  error_at(token->location, msg);
}

void error(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}
