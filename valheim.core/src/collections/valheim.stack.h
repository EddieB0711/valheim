#pragma once

#include "valheim.allocator.h"

typedef struct valheim_StackNode {
	void *value;
	struct valheim_StackNode *next;
} valheim_StackNode;

typedef struct valheim_Stack {
	u64 stride;
	valheim_StackNode *root;
} valheim_Stack;

VALHEIM_API b8 valheim_initStack(u64 stride, valheim_Allocator *allocator, valheim_Stack *stack);

VALHEIM_API void valheim_deinitStack(valheim_Stack *stack, valheim_Allocator *allocator);

VALHEIM_API b8 valheim_stackPush(valheim_Stack *stack, const void *value, valheim_Allocator *allocator);

VALHEIM_API b8 valheim_stackPop(valheim_Stack *stack, void *value, valheim_Allocator *allocator);