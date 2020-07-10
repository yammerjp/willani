#include "parse.h"

static FILE *parse_logfile;

void parse_log_open() {
  if (!is_printing_parse_log)
    return;

  parse_logfile = fopen("parse.log","w");
  if (!parse_logfile)
    error("fail to open parse.log");
}

void parse_log_close() {
  if (!is_printing_parse_log)
    return;

  fclose(parse_logfile);
}

void parse_log(char *s) {
  if (!is_printing_parse_log)
    return;

  fprintf(parse_logfile, "%s\n", s);
}
