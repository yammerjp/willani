#include "parse.h"

void new_tag(char *name, int namelen, Type *type) {
  for (Tag *tag = now_scope->tags; tag; tag = tag->next) {
    if (namelen == tag->namelen && strncmp(name, tag->name, namelen))
      error_at(name, "define a tag of struct is conflicted");
  }

  Tag *tag = calloc(1, sizeof(Tag));
  tag->name = name;
  tag->namelen = namelen;
  tag->type = type;
  tag->next = now_scope->tags;
  now_scope->tags = tag;
}

Tag *find_tag(char *name, int namelen, Tag *tags) {
  for (Tag *tag = tags; tag; tag = tag->next) {
    if (namelen == tag->namelen && strncmp(name, tag->name, namelen) == 0) {
      return tag;
    }
  }
  return NULL;
}
