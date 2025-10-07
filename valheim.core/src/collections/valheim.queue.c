#include "valheim.queue.h"
#include "valheim.memory.h"

b8 valheim_queueEnqueueT(void **front, void **back, u64 nodeStride, u64 valueStride, valheim_Allocator *allocator) {
	u8 *newNode = valheim_allocate(allocator, nodeStride);
	if (!newNode) {
		return false;
	}
	if (!*front) {
		*front = newNode;
		*back = *front;
	} else {
		u8 *next = (u8 *)*back + valueStride;
		valheim_copyMemory(next, &newNode, nodeStride);
		*back = newNode;
	}
	return true;
}

b8 valheim_queueDequeueT(void **front, void **back, u64 nodeStride, u64 valueStride, valheim_Allocator *allocator) {
	if (!*front) {
		return false;
	}
	u8 *current = *front;
	valheim_copyMemory(*front, (u8 *)(*front) + valueStride, nodeStride);
	valheim_free(allocator, current);
	return true;
}

b8 valheim_initQueue(u64 stride, valheim_Allocator *allocator, valheim_Queue *queue) {
	queue->stride = stride;
	queue->front = NULL;
	queue->back = NULL;
	return true;
}

void valheim_deinitQueue(valheim_Queue *queue, valheim_Allocator *allocator) {
	if (queue) {
		valheim_QueueNode *Current = queue->front;
		while (Current) {
			valheim_QueueNode *next = Current->next;
			valheim_free(allocator, Current->data);
			valheim_free(allocator, Current);
			Current = next;
		}
	}
}

b8 valheim_queueEnqueue(valheim_Queue *queue, const void *data, valheim_Allocator *allocator) {
	if (!queue) {
		return false;
	}

	valheim_QueueNode *node = valheim_allocate(allocator, sizeof(*node));
	node->data = valheim_allocate(allocator, queue->stride);
	node->next = NULL;

	valheim_copyMemory(node->data, data, queue->stride);

	if (!queue->front) {
		queue->front = node;
		queue->back = node;
	} else {
		queue->back->next = node;
		queue->back = node;
	}

	return true;
}

b8 valheim_queueDequeue(valheim_Queue *queue, valheim_Allocator *allocator, void *data) {
	if (queue && queue->front) {
		valheim_QueueNode *node = queue->front;
		queue->front = node->next;

		if (data) {
			valheim_copyMemory(data, node->data, queue->stride);
		}

		valheim_free(allocator, node->data);
		valheim_free(allocator, node);
		return true;
	}

	return false;
}

b8 valheim_queuePeek(valheim_Queue *queue, void *data) {
	if (queue && queue->front && data) {
		valheim_copyMemory(data, queue->front->data, queue->stride);
		return true;
	}

	return false;
}