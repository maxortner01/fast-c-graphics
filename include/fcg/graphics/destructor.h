#ifndef DESTRUCTOR_H_
#define DESTRUCTOR_H_

#include "../memory/stack.h"

typedef void (*FCG_Destructor)(FCG_Memory_Stack* FCG_CR);

typedef struct FCG_DestructorElement_s
{
    FCG_Destructor   handle;
    FCG_Memory_Stack arguments;
} FCG_DestructorElement;

#endif