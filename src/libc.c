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

// '1' => 1, '8' => 8, 'a' => 10, 'f' => 15
int ctoi(char c) {
  if ('0' <= c && c <= '9')
    return c - '0';
  if ('a' <= c && c <= 'f')
    return 10 + c - 'a';
  if ('A' <= c && c <= 'F')
    return 10 + c - 'A';
  return -1;
}

long strtol_mylibc(char *nptr, char **endptr, int base) {
  if (base < 0 || 16 < base) // base is 0 to 16
    return 0;

  int ret = 0;
  long sign_number = 1;

  // read white spaces
  while(isspace(*nptr))
    nptr++;

  // read +/- sign
  if(*nptr == '+') {
    nptr++;
  } else if(*nptr == '-') {
    nptr++;
    sign_number = -1;
  }

  // read "0" or "0x" or "0X"
  if ((base == 0 || base == 16) && *nptr == '0' && (nptr[1] == 'x' || nptr[1] == 'X')) {
    base = 16;
    nptr += 2;
  } else if (base == 0 && *nptr == '0') {
    base = 8;
    nptr++;
  }

  if (base == 0) {
    base = 10; // default base
  }

  // read digits
  for(;*nptr && nptr != *endptr; nptr++) {
    int d = ctoi(*nptr);
    if(d == -1 || d >= base)
      break;
    ret = ret * base + d;
  }
  *endptr = nptr;
  // FIXME: check overflow to return LONG_MAX
  return ret * sign_number;
}
