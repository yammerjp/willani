#include "parse.h"

Tag *tags = NULL;
Tag *outer_scope_tags = NULL;

void new_tag(char *name, int namelen, Type *type) {
  for (Tag *tag = tags; tag && tag != outer_scope_tags; tag = tag->next) {
    if (namelen == tag->namelen && strncmp(name, tag->name, namelen))
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
  for (Tag *tag = tags; tag; tag = tag->next) {
    if (namelen == tag->namelen && strncmp(name, tag->name, namelen) == 0 && tag->referable) {
      return tag;
    }
  }
  return NULL;
}
