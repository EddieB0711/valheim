#pragma once

#include "valheim.allocator.h"

typedef struct valheim_YamlNode {
	void *data;
	char *name;
	struct valheim_YamlNode *children;
	u32 childCount;
} valheim_YamlNode;

typedef struct valheim_Yaml {
	valheim_YamlNode *nodes;
} valheim_Yaml;

VALHEIM_API b8 valheim_loadYaml(const char *file, valheim_Allocator *allocator, valheim_Yaml *outYaml);