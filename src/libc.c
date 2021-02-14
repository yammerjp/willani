#include "libc.h"

int isspace(int c) {
  if (c == ' ' || c == '\f' || c == '\n' || c == '\r' || c == '\t' || c == '\v') {
    return 1;
  }
  return 0;
}

int isalpha(int c) {
  if ('a' <= c && c <= 'z' || 'A' <= c && c <= 'Z') {
    return 1;
  }
  return 0;
}

size_t strlen(char *s) {
  size_t ret = 0;
  while(*s++)
    ret++;
  return ret;
}

int strncmp(char *s1, char *s2, size_t n) {
  for(size_t i=0; i<n; i++) {
    if (s1[i] != s2[i] || s1[i] == 0) {
      return s1[i] - s2[i];
    }
  }
  return 0;
}

void *memcpy(void *dest, void *src, size_t n) {
//  unsigned char* d = (unsigned char*)dest;
//  unsigned char* s = (unsigned char*)src;
  char* d = (char*)dest;
  char* s = (char*)src;
  for(size_t i=0; i<n; i++) {
    d[i] = s[i];
  }
  return dest;
}

char *strstr(char *haystack, char *needle) {
  int i = 0;
  int j = 0;
  if(!needle[j])
    return haystack;
  while(haystack[i]) {
    j = 0;
    while(haystack[i+j] && haystack[i+j] == needle[j]) {
      j++;
      if(!needle[j])
        return &haystack[i];
    }
    i++;
  }
  return NULL;
}

void _exit(int); // The function  is written  with assembly.
void exit_myasm(int status) {
  _exit(status);
}
