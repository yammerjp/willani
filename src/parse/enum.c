#include "parse.h"

/*
typedef struct Enum Enum;
struct Enum {
  char *name;
  int namelen;
  Enum *next;
}
typedef struct EnumTag EnumTag;
struct EnumTag {
  char *name;
  int namelen;
  Enum *enms;
  EnumTag *next;
};
*/

void new_enum_tag(char *name, int namelen) {
  EnumTag *etag = calloc(1, sizeof(EnumTag));
  etag->name = name;
  etag->namelen = namelen;
  etag->next = now_scope->etags;
  now_scope->etags = etag;
}

void new_enum(char *name, int namelen, int num, EnumTag *etag) {
  Enum *enm = calloc(1, sizeof(Enum));
  enm->name = name;
  enm->namelen = namelen;
  enm->num = num;
  enm->next = etag->enms;
  etag->enms = enm;
}

Enum *find_in_enum_tag(char *name, int namelen, EnumTag *etag) {
  if (namelen==0)
    return NULL;
  for (Enum *enm = etag->enms; enm; enm = enm->next) {
    if (namelen == enm->namelen && !strncmp(name, enm->name, namelen))
      return enm;
  }
  return NULL;
}

Enum *find_in_enum_tags(char *name, int namelen, EnumTag *etags) {
  for (EnumTag *etag = etags; etag; etag = etag->next) {
    Enum *enm = find_in_enum_tag(name, namelen, etag);
    if (enm)
      return enm;
  }
  return NULL;
}

EnumTag *find_tag_in_enum_tags(char *name, int namelen, EnumTag *etags) {
  if (!namelen)
    return NULL;
  for (EnumTag *etag = etags; etag; etag = etag->next) {
    if (namelen == etag->namelen && !strncmp(name, etag->name, namelen))
      return etag;
  }
  return NULL;
}
