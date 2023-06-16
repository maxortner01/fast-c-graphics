#include <fcg/memory/stack.h>
#include "../assert.c"

void
FCG_Memory_InitializeStack(
    FCG_Memory_Stack* FCG_CR stack)
{
    memset(stack, 0, sizeof(FCG_Memory_Stack));
}

void 
FCG_Memory_DestroyStack(
    FCG_Memory_Stack* FCG_CR stack)
{
    /* Free the data and reset the memory */
    if (stack->data) free(stack->data);
    FCG_Memory_InitializeStack(stack);
}

void*
FCG_Memory_PushStack(
    FCG_Memory_Stack* FCG_CR stack,
    const FCG_Handle const data,
    CSIZE size)
{
    CU32 byte_offset = stack->top - (FCG_Iterator)stack->data;
    CU32 needed_size = byte_offset + size + sizeof(U32);

    if (needed_size >= stack->size)
    {
        CU32 new_size = (U32)(needed_size * 1.5f);

        /* Realloc */
        stack->data = realloc(stack->data, new_size);
        stack->top  = (FCG_Iterator)stack->data + byte_offset;
    }

    /* Copy the data over */
    void* position = (void*)stack->top;
    memcpy(stack->top, data, size);
    stack->top += size;
    memcpy(stack->top, &size, sizeof(U32));
    stack->top += sizeof(U32);

    stack->object_count++;
    stack->size += size;

    return position;
}

void*
FCG_Memory_StackTop(
    const FCG_Memory_Stack* FCG_CR stack)
{
    FCG_Iterator iterator = stack->top;
    iterator -= sizeof(U32);

    U32 top_byte_size = *iterator;
    iterator -= top_byte_size;

    return (void*)iterator;
}

void
FCG_Memory_PopStack(
    FCG_Memory_Stack* FCG_CR stack,
    void* FCG_CR dest)
{
    FCG_assert(stack->object_count);

    stack->object_count--;
    FCG_Iterator iterator = stack->top;
    iterator -= sizeof(U32);

    U32 top_byte_size = *iterator;
    iterator -= top_byte_size;
    
    stack->top   = iterator;
    stack->size -= top_byte_size;

    if (dest) memcpy(dest, iterator, top_byte_size);
}