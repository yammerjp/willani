#include "willani.h"

void error_at(Token *token, char *msg) {
  char *line = get_line_head(token->location, token->file);
  int line_num = get_line_number(line, token->file);
  char *end = get_line_end(token->location);

  // print filename and line number
  int indent = fprintf(stderr, "%s:%d: ",token->file->path, line_num);
  fprintf(stderr, "%.*s\n", (int)(end - line), line);

  // print error position
  int pos = token->location - line + indent;
  fprintf(stderr, "%*s", pos, "");
  fprintf(stderr, "^ %s\n", msg);
  exit_myasm(1);
}

void error(char *p) {
  fprintf(stderr, "%s\n", p);
  exit_myasm(1);
}
