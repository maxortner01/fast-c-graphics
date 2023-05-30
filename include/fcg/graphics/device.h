#ifndef DEVICE_H
#define DEVICE_H

#include "surface.h"
#include "../types.h"

typedef struct 
{
    const char* name;
    FCG_Handle  handle;
    FCG_Bool    active;
} FCG_RenderingDevice;

typedef struct 
{
    const char* name;
} FCG_GraphicsDevice;

typedef struct 
{
    const char*          name;
    FCG_Handle           handle;
    uint32_t             graphics_device_count;
    FCG_GraphicsDevice*  graphics_devices;
    FCG_Bool             active;
} FCG_Machine;

FCG_Result 
FCG_InitializeMachine(
    FCG_Machine* FCG_CR machine,
    const FCG_Surface* FCG_CR surface);

FCG_Result 
FCG_DestroyMachine(
    FCG_Machine* FCG_CR machine);

#endif