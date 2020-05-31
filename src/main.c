#include "willani.h"

char *user_input;

static void init(void) {
  type_int = new_type_int();
}

int main(int argc, char **argv) {
  if (argc != 2) {
    error("%s: invalid number of arguments.\n", argv[0]);
  }

  init();

  user_input = argv[1];
  Token *token = tokenize(user_input);

  // parse
  Function *func = program(token);

  parse_log(func);

  code_generate(func);

  return 0;
}
