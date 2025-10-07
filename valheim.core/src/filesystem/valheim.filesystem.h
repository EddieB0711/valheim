//
// Created by Eddie Boyle on 9/10/2025.
//

#ifndef VALHEIM_VALHEIM_FILESYSTEM_H
#define VALHEIM_VALHEIM_FILESYSTEM_H

#include "valheim.defines.h"

VALHEIM_API b8 valheim_readFile(const char *file, u64 *outSize, char *outContent);

#endif //VALHEIM_VALHEIM_FILESYSTEM_H