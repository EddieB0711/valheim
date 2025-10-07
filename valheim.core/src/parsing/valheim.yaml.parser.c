#include "valheim.yaml.parser.h"
#include "valheim.filesystem.h"

typedef enum valheim_YamlToken {
	valheim_YamlTokenProperty,
	valheim_YamlTokenSpace,
	valheim_YamlTokenColon,
	valheim_YamlTokenString,
	valheim_YamlTokenNumber,
	valheim_YamlTokenNewLine,
	valheim_YamlTokenDash,
	valheim_YamlTokenOpenBracket,
	valheim_YamlTokenCloseBracket
} valheim_YamlToken;

b8 valheim_loadYaml(const char *file, valheim_Allocator *allocator, valheim_Yaml *outYaml) {
	return false;
}
