#ifndef DEVICE_H
#define DEVICE_H

#include "../types.h"

#include "surface.h"
#include "destructor.h"

typedef struct FCG_GraphicsDevice_s
{
    char* name;
    FCG_Handle handle;
    I32        present_queue;
    I32        graphics_queue;
    FCG_Bool   discrete;
    U32 required_extensions_count;
    char** required_extensions;
} FCG_GraphicsDevice;

typedef struct FCG_RenderingDevice_s
{
    char* name;
    FCG_Handle  handle;
    FCG_Bool    active;
    FCG_GraphicsDevice* suitable_device;
} FCG_RenderingDevice;

typedef struct FCG_Machine_s
{
    const char*          name;
    FCG_Handle           handle;
    uint32_t             graphics_device_count;
    FCG_GraphicsDevice*  graphics_devices;
    FCG_Bool             active;
    FCG_Memory_Stack     destructor_stack;
#ifdef FCG_DEBUG
    FCG_Handle           debug;
#endif
} FCG_Machine;

FCG_SHARED
FCG_Result 
FCG_InitializeMachine(
    FCG_Machine* FCG_CR machine);

FCG_SHARED
FCG_Result 
FCG_DestroyMachine(
    FCG_Machine* FCG_CR machine);

#endif
