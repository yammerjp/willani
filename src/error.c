#include "willani.h"

void error_at(Token *token, char *msg) {
  char *location = token->location;
  char *line = get_line_head(location);
  int line_num = get_line_number(line);
  char *end = get_line_end(location);

  // print filename and line number
  int indent = fprintf(stderr, "%s:%d: ",token->filename, line_num);
  fprintf(stderr, "%.*s\n", (int)(end - line), line);

  // print error position
  int pos = location - line + indent;
  fprintf(stderr, "%*s", pos, "");
  fprintf(stderr, "^ %s\n", msg);
  exit(1);
}

void error(char *p) {
  fprintf(stderr, "%s\n", p);
  exit(1);
}
