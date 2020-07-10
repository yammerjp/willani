#include "willani.h"

char *filename;
char *user_input;
Function *functions;
int is_printing_ast = false;
int is_printing_asm_debug = false;
int is_printing_tokenize_log = false;
int is_printing_node_log = false;
int is_printing_parse_log = false;

int main(int argc, char **argv) {
  char *input_filename = NULL;
  char *output_filename = NULL;

  for (int i=1; i < argc; i++) {
    int len = strlen(argv[i]);
    if (len==2 && !strncmp("-o", argv[i], 2)
     || len==8 && !strncmp("--output", argv[i], 8)) {
      if (++i >= argc)
        error("Usage: %s inputfile -o outputfile", argv[0]);
      output_filename = argv[i];
      continue;
    }
    if (len==2 && !strncmp("-a", argv[i], 2)
     || len==5 && !strncmp("--ast", argv[i], 5)) {
      is_printing_ast = true;
      continue;
    }
    if (len==2 && !strncmp("-d", argv[i], 2)
     || len==7 && !strncmp("--debug", argv[i], 7)) {
      is_printing_asm_debug = true;
      continue;
    }
    if (len==3 && !strncmp("-tl", argv[i], 3)
     || len==14 && !strncmp("--tokenize-log", argv[i], 14)) {
      is_printing_tokenize_log = true;
      continue;
    }
    if (len==3 && !strncmp("-nl", argv[i], 3)
     || len==10 && !strncmp("--node-log", argv[i], 10)) {
      is_printing_node_log = true;
      continue;
    }
    if (len==3 && !strncmp("-pl", argv[i], 3)
     || len==11 && !strncmp("--parse-log", argv[i], 11)) {
      is_printing_parse_log = true;
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
