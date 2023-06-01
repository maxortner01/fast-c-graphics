#ifndef GDI_H
#define GDI_H

#include "../types.h"
#include "surface.h"
#include "device.h"

typedef struct 
{
    uint32_t             rendering_device_count;
    FCG_RenderingDevice* rendering_devices;
    FCG_Bool             active;
} FCG_GDI;

FCG_Result 
FCG_CreateGraphicsInstance(
    FCG_GDI* FCG_CR instance, 
    FCG_Surface* FCG_CR surface,
    FCG_Machine* FCG_CR machine);

FCG_Result 
FCG_DestroyGraphicsInstance(
    FCG_GDI* FCG_CR instance);


FCG_Result 
FCG_CreateRenderEnvironment(
    FCG_Surface* restrict surface, 
    FCG_Machine* restrict machine, 
    FCG_GDI* restrict gdi, 
    FCG_ContextType type, 
    FCG_DataHandle const data);

#endif