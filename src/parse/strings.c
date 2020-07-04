#include "parse.h"

String *strings;

char get_escape_char(char c) {
  switch (c) {
  case 'a': return '\a';
  case 'b': return '\b';
  case 't': return '\t';
  case 'n': return '\n';
  case 'v': return '\v';
  case 'f': return '\f';
  case 'r': return '\r';
  case 'e': return 27;
  case '0': return 0;
  default: return c;
  }
}

void add_string(String *s) {
  s->next = strings;
  strings = s;
}

String *new_string(char *p, int length) {
  char *head = calloc(length+1, sizeof(char));

  int offset = 0;
  for (int i=0; i<length; i++) {
    if (p[i] == '\\') {
      offset++;
      i++;
      head[i-offset] = get_escape_char(p[i]);
    } else {
      head[i-offset] = p[i];
    }
  }
  head[length-offset] = '\0';

  String *str = calloc(1, sizeof(String));
  str->p = head;
  str->length = length-offset+1;
  str->id = strings ? strings->id +1 : 0;
  return str;
}
