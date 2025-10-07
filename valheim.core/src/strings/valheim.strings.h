//
// Created by Eddie Boyle on 9/11/2025.
//

#ifndef VALHEIM_VALHEIM_STRINGS_H
#define VALHEIM_VALHEIM_STRINGS_H

#include "valheim.allocator.h"

VALHEIM_API void valheim_copyString(char *dst, u64 dstSize, const char *src);

VALHEIM_API u64 valheim_stringLength(const char *str);

VALHEIM_API char *valheim_duplicateString(const char *str, valheim_Allocator *allocator);

VALHEIM_API b8 valheim_stringsEqual(const char *str1, const char *str2);

VALHEIM_API void valheim_formatString(char *buffer, u64 bufferSize, const char *format, ...);

#endif //VALHEIM_VALHEIM_STRINGS_H
