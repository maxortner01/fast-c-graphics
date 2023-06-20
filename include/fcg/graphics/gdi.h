#ifndef GDI_H
#define GDI_H

#include "../types.h"
#include "surface.h"
#include "device.h"

typedef struct FCG_GDI_s
{
    uint32_t             rendering_device_count;
    FCG_RenderingDevice* rendering_devices;
    FCG_Bool             active;
} FCG_GDI;

FCG_SHARED
FCG_Result 
FCG_CreateGraphicsInstance(
    FCG_GDI* FCG_CR instance, 
    FCG_Surface* FCG_CR surface,
    FCG_Machine* FCG_CR machine);

FCG_SHARED
FCG_Result 
FCG_DestroyGraphicsInstance(
    FCG_GDI* FCG_CR instance);

FCG_SHARED
FCG_Result 
FCG_CreateRenderEnvironment(
    FCG_Surface* FCG_R surface, 
    FCG_Machine* FCG_R machine, 
    FCG_GDI* FCG_R gdi, 
    FCG_ContextType type, 
    FCG_DataHandle const data);

#endif