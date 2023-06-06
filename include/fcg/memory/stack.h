#ifndef STACK_H_
#define STACK_H_

#include "../types.h"

typedef struct
{
    U32          object_count;
    U32          size;
    FCG_Iterator top;
    FCG_Handle   data;
} FCG_Memory_Stack;

void
FCG_Memory_InitializeStack(
    FCG_Memory_Stack* FCG_CR stack);

void 
FCG_Memory_DestroyStack(
    FCG_Memory_Stack* FCG_CR stack);

void*
FCG_Memory_PushStack(
    FCG_Memory_Stack* FCG_CR stack,
    const FCG_Handle const data,
    CSIZE size);

void*
FCG_Memory_StackTop(
    const FCG_Memory_Stack* FCG_CR stack);

void
FCG_Memory_PopStack(
    FCG_Memory_Stack* FCG_CR stack,
    void* FCG_CR dest);

#endif