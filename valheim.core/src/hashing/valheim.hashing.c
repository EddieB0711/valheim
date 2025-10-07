//
// Created by Eddie Boyle on 9/9/2025.
//

#include "valheim.hashing.h"

u64 valheim_createHash(const u8 *data, u64 dataSize, u64 seed) {
	const u64 m = 0xc6a4a7935bd1e995ULL;
	const int r = 47;

	u64 h = seed ^ dataSize * m;

	const u64 *start = (const u64 *)data;
	const u64 *end = start + dataSize / 8;

	while (start != end) {
		u64 k = *start++;

		k *= m;
		k ^= k >> r;
		k *= m;

		h ^= k;
		h *= m;
	}

	const unsigned char *ptr = (const unsigned char *)start;

	switch (dataSize & 7) {
	case 7:
		h ^= (u64)ptr[6] << 48;
	case 6:
		h ^= (u64)ptr[5] << 40;
	case 5:
		h ^= (u64)ptr[4] << 32;
	case 4:
		h ^= (u64)ptr[3] << 24;
	case 3:
		h ^= (u64)ptr[2] << 16;
	case 2:
		h ^= (u64)ptr[1] << 8;
	case 1:
		h ^= (u64)ptr[0];
		h *= m;
	}

	h ^= h >> r;
	h *= m;
	h ^= h >> r;

	return h;
}
