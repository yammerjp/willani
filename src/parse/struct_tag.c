#include "parse.h"

void new_stag(char *name, int namelen, Type *type) {
  for (StructTag *stag = now_scope->stags; stag; stag = stag->next) {
    if (namelen == stag->namelen && !strncmp(name, stag->name, namelen)) {
      fprintf(stderr, "tag name:%.*s", namelen, name);
      error("define a struct tag is conflicted");
    }
  }

  StructTag *stag = calloc(1, sizeof(StructTag));
  stag->name = name;
  stag->namelen = namelen;
  stag->type = type;
  stag->next = now_scope->stags;
  now_scope->stags = stag;
}

StructTag *find_stag_in_stags(char *name, int namelen, StructTag *stags) {
  for (StructTag *stag = stags; stag; stag = stag->next) {
    if (namelen == stag->namelen && strncmp(name, stag->name, namelen) == 0) {
      return stag;
    }
  }
  return NULL;
}
