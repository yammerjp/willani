#include "willani.h"

char *filename;
char *user_input;
Function *functions;
int is_printing_ast = false;
int is_printing_asm_debug = false;
int is_printing_tokenize_log = false;
int is_printing_node_log = false;
int is_printing_parse_log = false;

static void help(char * argv0);

int main(int argc, char **argv) {
  char *input_filename = NULL;
  char *output_filename = NULL;

  for (int i=1; i < argc; i++) {
    int len = strlen(argv[i]);
    if (len==2 && !strncmp("-h", argv[i], 2)
     || len==6 && !strncmp("--help", argv[i], 6)) {
      help(argv[0]);
    }

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
    if (argv[i][0] == '-')
      error("unknown option: %s", argv[i]);
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

static void help(char *argv0) {
  fprintf(stderr, "Willani is a compiler for the C programming language.\n");
  fprintf(stderr, "Willani output a text of assembly language of AT&T syntax\n\n");
  fprintf(stderr, "Usage: %s [options] <file>\n", argv0);
  fprintf(stderr, "  <file> : input file of C programming language\n");
  fprintf(stderr, "Options\n");
  fprintf(stderr, "  -o <file>\n");
  fprintf(stderr, "  --output <file>  Place the output into <file>\n");
  fprintf(stderr, "  -d\n");
  fprintf(stderr, "  --debug          Output debug info into <file> (with using -o option)\n");
  fprintf(stderr, "  -a\n");
  fprintf(stderr, "  --ast            Output ast.json to print Abstruct Syntax Tree\n");
  fprintf(stderr, "  -tl\n");
  fprintf(stderr, "  --tokenize-log   Output tokenize.log to print tokenizing log\n");
  fprintf(stderr, "  -nl\n");
  fprintf(stderr, "  --node-log       Output node.log to print parsed node\n");
  fprintf(stderr, "  -pl\n");
  fprintf(stderr, "  --parse-log      Output parse.log to print parsing log\n");
  exit(0);
}
