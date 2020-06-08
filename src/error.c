#include "willani.h"

void error_at(Token *token, char *msg) {
  char *line = get_line_head(token);
  int line_num = get_line_number(line);
  char *end = get_line_end(token);

  // print filename and line number
  int indent = fprintf(stderr, "%s:%d: ",filename, line_num);
  fprintf(stderr, "%.*s\n", (int)(end-line), line);

  // print error position
  int pos = token->location - line + indent;
  fprintf(stderr, "%*s", pos, "");
  fprintf(stderr, "^ %s\n", msg);
  exit(1);
}

void error(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}
