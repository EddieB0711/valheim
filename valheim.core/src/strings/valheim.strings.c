//
// Created by Eddie Boyle on 9/11/2025.
//

#include "valheim.strings.h"

#include <string.h>
#include <stdarg.h>
#include <stdio.h>

void valheim_copyString(char *dst, u64 dstSize, const char *src) {
  strcpy_s(dst, dstSize, src);
}

u64 valheim_stringLength(const char *str) {
  return strlen(str);
}

char *valheim_duplicateString(const char *str, valheim_Allocator *allocator) {
  const u64 length = valheim_stringLength(str);

  char *buffer = valheim_allocate(allocator, length + 1);
  strcpy_s(buffer, length + 1, str);

  buffer[length] = '\0';

  return buffer;
}

b8 valheim_stringsEqual(const char *str1, const char *str2) {
  return strcmp(str1, str2) == 0;
}

void valheim_formatString(char *buffer, u64 bufferSize, const char *format, ...) {
  va_list args;
  va_start(args, format);
  vsnprintf(buffer, bufferSize, format, args);
  va_end(args);
}
