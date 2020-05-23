#include "willani.h"

char *user_input;

int main(int argc, char **argv) {
  if (argc != 2) {
    error("%s: invalid number of arguments.\n", argv[0]);
  }

  user_input = argv[1];
  Token *token = tokenize(user_input);

  // parse
  Node *node = program(&token, token);

  int offset = 0;
  for ( LVar *lvar = locals; lvar; lvar = lvar->next ) {
    offset += 8;
    lvar->offset = offset;
  }
  parse_log(node);

  code_generate(node);

  return 0;
}
