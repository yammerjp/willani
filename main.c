#include "willani.h"

char *user_input;

int main(int argc, char **argv) {
  if (argc != 2) {
    error("%s: invalid number of arguments.\n", argv[0]);
  }

  user_input = argv[1];
  Token *token = tokenize(user_input);

  // parse
  Node *node = expr(&token, token);

  parse_log(node);

  printf(".intel_syntax noprefix\n");
  printf(".globl main\n");
  printf("main:\n");

  code_generate(node);

  printf("  pop rax\n");
  printf("  ret\n");
  return 0;
}
