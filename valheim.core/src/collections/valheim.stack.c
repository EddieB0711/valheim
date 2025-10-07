#include "valheim.stack.h"

#include "valheim.memory.h"

b8 valheim_initStack(u64 stride, valheim_Allocator *allocator, valheim_Stack *stack) {
	stack->stride = stride;
	stack->root = NULL;
	return true;
}

void valheim_deinitStack(valheim_Stack *stack, valheim_Allocator *allocator) {
	if (stack) {
		valheim_StackNode *current = stack->root;
		while (current) {
			valheim_StackNode *next = current->next;
			valheim_free(allocator, current->value);
			valheim_free(allocator, current);
			current = next;
		}
		valheim_free(allocator, stack);
	}
}

b8 valheim_stackPush(valheim_Stack *stack, const void *value, valheim_Allocator *allocator) {
	valheim_StackNode *newNode = valheim_allocate(allocator, sizeof *newNode);
	newNode->value = valheim_allocate(allocator, stack->stride);

	valheim_copyMemory(newNode->value, value, stack->stride);

	newNode->next = stack->root;
	stack->root = newNode;
	return true;
}

b8 valheim_stackPop(valheim_Stack *stack, void *value, valheim_Allocator *allocator) {
	if (stack && stack->root) {
		valheim_StackNode *root = stack->root;
		stack->root = root->next;
		valheim_copyMemory(value, root->value, stack->stride);
		valheim_free(allocator, root->value);
		valheim_free(allocator, root);
		return true;
	}

	return false;
}
