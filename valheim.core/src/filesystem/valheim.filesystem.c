//
// Created by Eddie Boyle on 9/10/2025.
//

#include "valheim.filesystem.h"

#include <stdio.h>

b8 valheim_readFile(const char *file, u64 *outSize, char *outContent) {
	FILE *stream = fopen(file, "rb");
	if (!stream) {
		return false;
	}

	fseek(stream, 0, SEEK_END);
	u64 fileSize = ftell(stream);
	fseek(stream, 0, SEEK_SET);

	if (outSize) {
		*outSize = fileSize;
	}

	fread(outContent, 1, fileSize, stream);
	fclose(stream);
	return true;
}
