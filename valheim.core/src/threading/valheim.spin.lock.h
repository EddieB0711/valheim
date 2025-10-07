#pragma once

#include "valheim.defines.h"

typedef struct valheim_SpinLock {
	volatile u64 val;
} valheim_SpinLock;

VALHEIM_API void valheim_initSpinLock(valheim_SpinLock *lock);

VALHEIM_API void valheim_acquireSpinLock(valheim_SpinLock *lock);

VALHEIM_API void valheim_releaseSpinLock(valheim_SpinLock *lock);