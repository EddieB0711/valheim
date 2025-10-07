#pragma once

#include "valheim.allocator.h"
#include "valheim.memory.h"

#define valheim_QueueD(T) \
	struct { \
		struct {\
			T value;\
			void *next; \
		} *front;\
		struct {\
			T value;\
			void *next; \
		} *back;\
	}

#define valheim_initQueueD(q) \
	((q).front = NULL, \
	((q).back = NULL))

#define valheim_expandQueue(q) &(q).front, &(q).back, sizeof(*(q).front)

#define valheim_queueEmpty(q) (q).front ? true : false

#define valheim_queueEnqueueD(q, val, alloc) \
	(valheim_queueEnqueueT(valheim_expandQueue(q), sizeof(val), alloc) ? ((q).back->value = (val), true) : false)

#define valheim_queueDequeueD(q, val, alloc) \
	(valheim_queueEmpty(q) ? (printf("empty"), false) : (printf("not-empty"), true), true)

typedef struct valheim_QueueNode {
	void *data;
	struct valheim_QueueNode *next;
} valheim_QueueNode;

typedef struct valheim_Queue {
	u64 stride;
	valheim_QueueNode *front;
	valheim_QueueNode *back;
} valheim_Queue;

VALHEIM_API b8 valheim_queueEnqueueT(void **front, void **back, u64 nodeStride, u64 valueStride, valheim_Allocator *allocator);

VALHEIM_API b8 valheim_queueDequeueT(void **front, void **back, u64 nodeStride, u64 valueStride, valheim_Allocator *allocator);

VALHEIM_API b8 valheim_initQueue(u64 stride, valheim_Allocator *allocator, valheim_Queue *queue);

VALHEIM_API void valheim_deinitQueue(valheim_Queue *queue, valheim_Allocator *allocator);

VALHEIM_API b8 valheim_queueEnqueue(valheim_Queue *queue, const void *data, valheim_Allocator *allocator);

VALHEIM_API b8 valheim_queueDequeue(valheim_Queue *queue, valheim_Allocator *allocator, void *data);

VALHEIM_API b8 valheim_queuePeek(valheim_Queue *queue, void *data);
