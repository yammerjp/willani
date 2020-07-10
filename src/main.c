#include "willani.h"

char *filename;
char *user_input;
Function *functions;

int main(int argc, char **argv) {
  char *input_filename = NULL;
  char *output_filename = NULL;

  for (int i=1; i < argc; i++) {
    int len = strlen(argv[i]);
    if (len==2 && !strncmp("-o", argv[i], 2)) {
      if (++i >= argc)
        error("Usage: %s inputfile -o outputfile", argv[0]);
      output_filename = argv[i];
      continue;
    }
    if (input_filename != NULL)
      error("duplicated input files");
    input_filename = argv[i];
    filename = argv[i];
  }
  if (!output_filename || !input_filename)
    error("Usage: %s inputfile -o outputfile", argv[0]);

  user_input = read_file(input_filename);
  Token *token = tokenize(user_input, input_filename);
  token = preprocess(token);

  // parse
  program(token);

  code_generate(output_filename);

  return 0;
}
