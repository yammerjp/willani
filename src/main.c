#include "willani.h"

char *filename;
char *user_input;
Function *functions;

int main(int argc, char **argv) {
  if (argc != 2)
    error("%s: invalid number of arguments.\n", argv[0]);

  filename = argv[1];
  user_input = read_file(filename);
  Token *token = tokenize(user_input);
  token = preprocess(token);

  // parse
  program(token);

  code_generate();

  return 0;
}
