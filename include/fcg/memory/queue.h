#ifndef QUEUE_H_
#define QUEUE_H_

#include "../types.h"

typedef struct FCG_Memory_Queue_s
{
    U32          object_count;
    U32          size;
    U32          allocated_amount;
    FCG_Iterator back;
    FCG_Handle   data;
} FCG_Memory_Queue;

FCG_SHARED
void
FCG_Memory_InitializeQueue(
    FCG_Memory_Queue* FCG_CR queue);

FCG_SHARED
void 
FCG_Memory_DestroyQueue(
    FCG_Memory_Queue* FCG_CR queue);

FCG_SHARED
void*
FCG_Memory_PushQueue(
    FCG_Memory_Queue* FCG_CR queue,
    const FCG_Handle const data,
    CSIZE size);

FCG_SHARED
void*
FCG_Memory_QueueFront(
    const FCG_Memory_Queue* FCG_CR queue);

FCG_SHARED
void*
FCG_Memory_QueueGet(
    FCG_Memory_Queue* FCG_CR queue,
    CU32                     index);

#endif