#include "parse.h"

TypeDef *find_in_typedefs(char *name, int namelen, TypeDef *tdfs) {
  for (TypeDef* tdf = tdfs; tdf; tdf = tdf->next) {
    if (namelen == tdf->namelen && !strncmp(name, tdf->name, namelen))
      return tdf;
  }
  return NULL;
}

void new_typedef(Type *type, char *name, int namelen) {
  if (type->is_static)
    error("not support static variables of typedef");
  TypeDef *tdf = calloc(1, sizeof(TypeDef));
  tdf->type = type;
  tdf->name = name;
  tdf->namelen = namelen;
  tdf->next = now_scope->tdfs;

  now_scope->tdfs = tdf;
}
