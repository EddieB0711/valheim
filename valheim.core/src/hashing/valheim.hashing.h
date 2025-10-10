//
// Created by Eddie Boyle on 9/9/2025.
//

#ifndef VALHEIM_VALHEIM_HASHING_H
#define VALHEIM_VALHEIM_HASHING_H

#include "valheim.defines.h"

VALHEIM_API u64 valheim_hash(const u8* data, u64 dataSize, u64 seed);

#endif //VALHEIM_VALHEIM_HASHING_H