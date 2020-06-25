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

Var *find_var_now_scope(char *name, int namelen, IncludeTypedef include_typedef) {
  for (Scope *sc = now_scope; sc; sc = sc->parent) {
    Var *var = find_var(name, namelen, sc->vars, NULL, include_typedef);
    if (var)
      return var;
  }
  return NULL;
}

Tag *find_tag_now_scope(char *name, int namelen) {
  for (Scope *sc = now_scope; sc; sc = sc->parent) {
    Tag *tag = find_tag(name, namelen, sc->tags);
    if (tag)
      return tag;
  }
  return NULL;
}
