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
    if (find_in_typedefs(name, namelen, sc->tdfs))
      return NULL;
    Var *var = find_in_vars(name, namelen, sc->vars);
    if (var)
      return var;
  }
  return NULL;
}

TypeDef *find_typedef(char *name, int namelen) {
  for (Scope *sc = now_scope; sc; sc = sc->parent) {
    if (find_in_vars(name, namelen, sc->vars))
      return NULL;
    TypeDef *tdf = find_in_typedefs(name, namelen, sc->tdfs);
    if (tdf)
      return tdf;
  }
  return NULL;
}

StructTag *find_tag(char *name, int namelen) {
  for (Scope *sc = now_scope; sc; sc = sc->parent) {
    StructTag *tag = find_tag_in_tags(name, namelen, sc->tags);
    if (tag)
      return tag;
  }
  return NULL;
}
