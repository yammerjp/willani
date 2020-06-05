#include "parse.h"

String *strings;
String *new_string(char *p, int length) {
  String *str = calloc(1, sizeof(String));
  str->p = p;
  str->length = length;
  str->id = strings ? strings->id +1 : 0;
  str->next = strings;

  strings = str;
}
