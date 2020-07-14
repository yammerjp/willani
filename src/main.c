#include "willani.h"

Function *functions;
int is_printing_ast = false;
int is_printing_token = false;
int is_printing_asm_debug = false;
int is_printing_tokenize_log = false;
int is_printing_node_log = false;
int is_printing_parse_log = false;
static char *input_filename = NULL;
static char *output_filename = NULL;

static void help(char * argv0);

bool find_strs(char *target, char* str1, char* str2) {
  int len = strlen(target);
  if (len == strlen(str1) && !strncmp(target, str1, len))
    return true;
  if (len == strlen(str2) && !strncmp(target, str2, len))
    return true;
  return false;
}
void usage(char *argv0) {
  fprintf(stderr, " Usage: %s inputfile -o outputfile", argv0);
  exit(1);
}

void read_args(int argc, char **argv) {
  for (int i=1; i < argc; i++) {
    if (find_strs(argv[i], "-h", "--help")) {
      help(argv[0]);
    }
    if (find_strs(argv[i], "-o", "--output")) {
      if (++i >= argc)
        usage(argv[0]);
      output_filename = argv[i];
      continue;
    }
    if (find_strs(argv[i], "-aj", "--ast-json")) {
      is_printing_ast = true;
      continue;
    }
    if (find_strs(argv[i], "-tj", "--token-json")) {
      is_printing_token = true;
      continue;
    }
    if (find_strs(argv[i], "-d", "--debug")) {
      is_printing_asm_debug = true;
      continue;
    }
    if (find_strs(argv[i], "-tl", "--tokenize-log")) {
      is_printing_tokenize_log = true;
      continue;
    }
    if (find_strs(argv[i], "-nl", "--node-log")) {
      is_printing_node_log = true;
      continue;
    }
    if (find_strs(argv[i], "-pl", "--parse-log")) {
      is_printing_parse_log = true;
      continue;
    }
    if (argv[i][0] == '-') {
      fprintf(stderr, "unknown option: %s", argv[i]);
      exit(1);
    }

    if (input_filename != NULL)
      error("duplicated input files");
    input_filename = argv[i];
  }

  if (!output_filename || !input_filename)
    usage(argv[0]);
}

int main(int argc, char **argv) {
  // read commandline arguments and flag options
  read_args(argc, argv);

  // for debug
  tokenize_log_open();

  SourceFile *sf = read_file(input_filename);
  Token *token = tokenize(sf);
  token = preprocess(token);

  // for debug
  tokenize_log_close();

  token_json_log(token);

  // parse
  program(token);

  code_generate(output_filename);
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
