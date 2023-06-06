#include <fcg/memory/queue.h>
#include "../assert.c"

void
FCG_Memory_InitializeQueue(
    FCG_Memory_Queue* FCG_CR queue)
{
    memset(queue, 0, sizeof(FCG_Memory_Queue));
}

void 
FCG_Memory_DestroyQueue(
    FCG_Memory_Queue* FCG_CR queue)
{
    /* Free the data and reset the memory */
    if (queue->data) free(queue->data);
    FCG_Memory_InitializeQueue(queue);
}

void*
FCG_Memory_PushQueue(
    FCG_Memory_Queue* FCG_CR queue,
    const FCG_Handle const data,
    CSIZE size)
{
    CU32 byte_offset = queue->back - (FCG_Iterator)queue->data;
    CU32 needed_size = byte_offset + size + sizeof(U32);

    if (needed_size >= queue->size)
    {
        CU32 new_size = (U32)(needed_size * 1.5f);

        /* Realloc */
        queue->data = realloc(queue->data, new_size);
        FCG_assert(queue->data);

        queue->back = (FCG_Iterator)queue->data + byte_offset;
        queue->allocated_amount = new_size;
    }

    /* Copy the data over */
    memcpy(queue->back, &size, sizeof(U32));
    queue->back += sizeof(U32);
    
    void* position = (void*)queue->back;
    memcpy(queue->back, data, size);
    queue->back += size;

    queue->object_count++;
    queue->size += size;

    return position;
}

void*
FCG_Memory_QueueFront(
    const FCG_Memory_Queue* FCG_CR queue)
{
    FCG_Iterator iterator = queue->data;
    iterator += sizeof(U32);

    return (void*)iterator;
}

void*
FCG_Memory_QueueGet(
    FCG_Memory_Queue* FCG_CR queue,
    CU32                     index)
{
    FCG_assert(index < queue->object_count);

    FCG_Iterator iterator = queue->data;
    if (!index) return iterator + sizeof(U32);

    for (uint32_t i = 0; i < index; i++)
    {
        U32 bytes = *(U32*)iterator;
        iterator += sizeof(U32) + bytes;
    }

    return iterator + sizeof(U32);
}