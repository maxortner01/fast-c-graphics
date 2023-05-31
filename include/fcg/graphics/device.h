#ifndef DEVICE_H
#define DEVICE_H

#include "../memory/stack.h"
#include "surface.h"
#include "../types.h"

#define DEBUG

typedef void (*FCG_Destructor)(FCG_Memory_Stack* FCG_CR);

typedef struct 
{
    char* name;
    FCG_Handle  handle;
    FCG_Bool    active;
} FCG_RenderingDevice;

typedef struct 
{
    char* name;
    FCG_Handle handle;
    I32        present_queue;
    I32        graphics_queue;
    FCG_Bool   discrete;
    U32 required_extensions_count;
    char** required_extensions;
} FCG_GraphicsDevice;

typedef struct
{
    FCG_Destructor   handle;
    FCG_Memory_Stack arguments;
} FCG_DestructorElement;

typedef struct 
{
    const char*          name;
    FCG_Handle           handle;
    uint32_t             graphics_device_count;
    FCG_GraphicsDevice*  graphics_devices;
    FCG_Bool             active;
    FCG_Memory_Stack     destructor_stack;
#ifdef DEBUG
    FCG_Handle           debug;
#endif
} FCG_Machine;

FCG_Result 
FCG_InitializeMachine(
    FCG_Machine* FCG_CR machine,
    FCG_Surface* FCG_CR surface);

FCG_Result 
FCG_DestroyMachine(
    FCG_Machine* FCG_CR machine);

#endif