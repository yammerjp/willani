#include "willani.h"

// "0x02" => 2, "2" => 2, "0b10" => 2, "010" => 8
long str_to_l(char *p, int length) {
  int num;
  char *p_end;
  if (p[1] == 'b' || p[1] == 'B') {
    if (length == 2) {
      fprintf(stderr, "token: %.*s\n", length, p);
      error("failed to read number token");
    }
    num = strtol(p+2, &p_end, 2);
  } else {
    num = strtol(p, &p_end, 0);
  }
  if (p+length != p_end) {
    fprintf(stderr, "token: %.*s\n", length, p);
    error("failed to read number token");
  }
  return num;
}
