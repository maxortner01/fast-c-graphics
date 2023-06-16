#include <fcg/graphics/gdi.h>
#include "../../assert.c"

FCG_Result 
FCG_CreateGraphicsInstance(
    FCG_GDI* FCG_CR instance,  
    const FCG_Machine* FCG_CR machine)
{
    memset(instance, 0, sizeof(FCG_GDI));
    instance->active = FCG_True;

    // create rendering device from the physical devices given in the machine

    // Create logic device

    return FCG_SUCCESS;
}

FCG_Result 
FCG_DestroyGraphicsInstance(
    FCG_GDI* FCG_CR instance)
{
    FCG_assert(instance->active);

    if (instance->rendering_devices) free(instance->rendering_devices);

    memset(instance, 0, sizeof(FCG_GDI));
    return FCG_SUCCESS;
}