#include "parse.h"

void new_tag(char *name, int namelen, Type *type) {
  for (StructTag *tag = now_scope->tags; tag; tag = tag->next) {
    if (namelen == tag->namelen && strncmp(name, tag->name, namelen))
      error_at(name, "define a tag of struct is conflicted");
  }

  StructTag *tag = calloc(1, sizeof(StructTag));
  tag->name = name;
  tag->namelen = namelen;
  tag->type = type;
  tag->next = now_scope->tags;
  now_scope->tags = tag;
}

StructTag *find_tag_in_tags(char *name, int namelen, StructTag *tags) {
  for (StructTag *tag = tags; tag; tag = tag->next) {
    if (namelen == tag->namelen && strncmp(name, tag->name, namelen) == 0) {
      return tag;
    }
  }
  return NULL;
}
