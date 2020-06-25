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
