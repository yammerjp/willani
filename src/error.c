#include "willani.h"

void error_at_token(Token *prevtoken, char *msg) {
  int position = (prevtoken->location + prevtoken->length) - user_input;
  fprintf(stderr, "%s\n", user_input);
  fprintf(stderr, "%*s", position, "");
  fprintf(stderr, "^ ");

  fprintf(stderr, "%s\n", msg);
  exit(1);
}

void error(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}
