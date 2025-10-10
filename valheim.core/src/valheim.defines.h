#ifndef VALHEIM_VALHEIM_DEFINES_H
#define VALHEIM_VALHEIM_DEFINES_H
 
#include <stdint.h>
#include <stdbool.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef float f32;
typedef double f64;

typedef bool b8;

#define VALHEIM_ARRAY_LEN(x) (sizeof(x) / sizeof((x)[0]))

#define VALHEIM_DEFAULT_ALIGN (sizeof(void*) * 2)

#define VALHEIM_KIBIBYTE(x) ((x) * 1024)
#define VALHEIM_MIBIBYTE(x) ((x) * 1024 * 1024)
#define VALHEIM_GIBIBYTE(x) ((x) * 1024 * 1024 * 1024)

#ifdef ODIN_EXPORT
#define VALHEIM_API __declspec(dllexport)
#else
#define VALHEIM_API __declspec(dllimport)
#endif

#endif //VALHEIM_VALHEIM_DEFINES_H