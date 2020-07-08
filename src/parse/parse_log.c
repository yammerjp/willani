#include "parse.h"

FILE *parse_logfile;

void parse_log_open() {
  parse_logfile = fopen("parse.log","w");
  if (!parse_logfile)
    error("fail to open parse.log");
}

void parse_log_close() {
  fclose(parse_logfile);
}

void parse_log(char *s) {
  fprintf(parse_logfile, "%s\n", s);
}
