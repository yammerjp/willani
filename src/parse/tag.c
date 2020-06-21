#include "parse.h"

Tag *tags = NULL;
Tag *outer_scope_tags = NULL;

void new_tag(char *name, int namelen, Type *type) {
  for (Tag *cur = tags; cur && cur != outer_scope_tags; cur = cur->next) {
    if (namelen == cur->namelen && strncmp(name, cur->name, namelen))
      error_at(name, "define a tag of struct is conflicted");
  }

  Tag *tag = calloc(1, sizeof(Tag));
  tag->name = name;
  tag->namelen = namelen;
  tag->type = type;
  tag->referable = true;
  tag->next = tags;
  tags = tag;
}

Tag *find_tag(char *name, int namelen) {
  fprintf(stderr, " finding tag: %.*s\n", namelen, name);
  for (Tag *cur = tags; cur; cur = cur->next) {
    fprintf(stderr, " tag: %.*s %s\n", cur->namelen, cur->name, cur->referable ? "(referable)" : "");
    if (namelen == cur->namelen && strncmp(name, cur->name, namelen) == 0 && cur->referable) {
      fprintf(stderr, "found tag\n");
      return cur;
    }
  }
  fprintf(stderr, "not found tag\n");
  return NULL;
}

void unrefer_outer_scope_tags() {
  for (Tag *cur = tags; cur && cur != outer_scope_tags; cur = cur->next)
    cur->referable = false;
}
