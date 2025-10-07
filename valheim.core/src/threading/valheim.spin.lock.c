#include "valheim.spin.lock.h"

#include <Windows.h>

void valheim_initSpinLock(valheim_SpinLock *lock) {
	lock->val = 0;
}

void valheim_acquireSpinLock(valheim_SpinLock *lock) {
	while (InterlockedExchangeAcquire(&lock->val, 1)) {
		while (lock->val) {
			YieldProcessor();
		}
	}
}

void valheim_releaseSpinLock(valheim_SpinLock *lock) {
	InterlockedExchange(&lock->val, 0);
}
