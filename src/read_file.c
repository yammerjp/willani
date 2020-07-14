#include "willani.h"

SourceFile *read_file(char *path) {
  FILE *fp = fopen(path, "r");
  if (!fp) {
    fprintf(stderr, "file :%s", path);
    error("cannot open");
  }

  // measure file length
  if (fseek(fp, 0, SEEK_END) == -1)
    error("fseek is failed");
  size_t size = ftell(fp);
  if (fseek(fp, 0, SEEK_SET) == -1)
    error("fseek is failed");

  // read file
  char *buf = calloc(1, size + 2);
  fread(buf, size, 1, fp);

  // add '\n\0' to tail if it is not exist
  if (size == 0 ||  buf[size - 1] != '\n')
    buf[size++] = '\n';
  buf[size] = '\0';

  fclose(fp);
  SourceFile *sf = calloc(1, sizeof(SourceFile));
  sf->path = path;
  sf->text = buf;
  return sf;
}
