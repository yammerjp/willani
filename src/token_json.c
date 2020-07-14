#include "willani.h"

void token_json_log(Token *token) {
  FILE *fp = fopen("token.json","w");
  if (!fp)
    error("fail to open token.json");

  fprintf(fp, "[\n");
  while (token) {
    fprintf(fp, "  ");
    fprint_ast_token(fp, token, 2);
    token = token->next;
    if (token)
      fprintf(fp, ",\n");
  }
  fprintf(fp, "\n]\n");

  fclose(fp);
}
