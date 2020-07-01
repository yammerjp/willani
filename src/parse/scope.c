#include "parse.h"

Scope *now_scope;

void scope_in() {
  Scope *sc = calloc(1, sizeof(Scope));
  sc->parent = now_scope;
  now_scope = sc;
}

void scope_out() {
  now_scope = now_scope->parent;
}

Var *find_var(char *name, int namelen) {
  for (Scope *sc = now_scope; sc; sc = sc->parent) {
    if (find_in_typedefs(name, namelen, sc->tdfs) || find_in_enum_tags(name, namelen, sc->etags))
      return NULL;
    Var *var = find_in_vars(name, namelen, sc->vars);
    if (var)
      return var;
  }
  return NULL;
}

TypeDef *find_typedef(char *name, int namelen) {
  for (Scope *sc = now_scope; sc; sc = sc->parent) {
    if (find_in_vars(name, namelen, sc->vars) || find_in_enum_tags(name, namelen, sc->etags))
      return NULL;
    TypeDef *tdf = find_in_typedefs(name, namelen, sc->tdfs);
    if (tdf)
      return tdf;
  }
  return NULL;
}

StructTag *find_stag(char *name, int namelen) {
  for (Scope *sc = now_scope; sc; sc = sc->parent) {
    StructTag *stag = find_stag_in_stags(name, namelen, sc->stags);
    if (stag)
      return stag;
  }
  return NULL;
}

Enum *find_enum(char *name, int namelen) {
  for (Scope *sc = now_scope; sc; sc = sc->parent) {
    if (find_in_vars(name, namelen, sc->vars) || find_in_typedefs(name, namelen, sc->tdfs))
      return NULL;
    Enum *enm = find_in_enum_tags(name, namelen, sc->etags);
    if (enm)
      return enm;
  }
  return NULL;
}

EnumTag *find_enum_tag(char *name, int namelen) {
  for (Scope *sc = now_scope; sc; sc = sc->parent) {
    EnumTag *etag = find_tag_in_enum_tags(name, namelen, sc->etags);
    if (etag)
      return etag;
  }
  return NULL;
}
