#ifndef BUFFER_H_
#define BUFFER_H_

#include "../memory/queue.h"
#include "device.h"

typedef struct FCG_Data_BufferLayout_s
{
    // To be replaced with general block allocator object
    FCG_Memory_Queue buffers;
} FCG_Data_BufferLayout;

typedef struct FCG_Data_Attribute_s
{
    U32 element_count;
    U32 element_size;
} FCG_Data_Attribute;

typedef struct FCG_Data_Buffer_s
{
    FCG_Handle owner; // pointer to allocator that created it

    FCG_Handle buffer;
    FCG_Handle allocation;

    void* data;
    U32   allocated;
    U32   attribute_count;
    FCG_Data_Attribute* attributes;
} FCG_Data_Buffer;

FCG_Result
FCG_Data_InitLayout(
    FCG_Data_BufferLayout* FCG_CR layout);

FCG_Result
FCG_Data_DestroyLayout(
    FCG_Data_BufferLayout* FCG_CR layout);

FCG_Data_Buffer*
FCG_Data_CreateBuffer(
    const FCG_Machine* FCG_CR machine,
    FCG_Data_BufferLayout* FCG_CR layout);

void
FCG_Data_LoadData(
    FCG_Data_Buffer* FCG_CR buffer,
    void* data,
    CU32  block_size,
    U32   attribute_count,
    FCG_Data_Attribute* attributes);

FCG_Result
FCG_Data_DestroyBuffer(
    FCG_Data_Buffer* FCG_CR buffer);

#endif