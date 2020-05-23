#include "willani.h"

void error_at(Token *prevtoken, char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);

  int position = (prevtoken->location + prevtoken->length) - user_input;
  fprintf(stderr, "%s\n", user_input);
  fprintf(stderr, "%*s", position, "");
  fprintf(stderr, "^ ");

  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

void error(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}
