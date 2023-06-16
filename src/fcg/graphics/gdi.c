#include <fcg/graphics/gdi.h>
#include "../assert.c"

FCG_Result 
FCG_CreateRenderEnvironment(
    FCG_Surface* restrict surface, 
    FCG_Machine* restrict machine, 
    FCG_GDI* restrict gdi, 
    FCG_ContextType type, 
    FCG_DataHandle const data)
{
    FCG_Result result;
    result = FCG_Surface_Create(surface, type, data);
    if (result) return result;

    result = FCG_InitializeMachine(machine);
    if (result) return result;

    result = FCG_CreateGraphicsInstance(gdi, surface, machine);
    if (result) return result;

    /* Add call to FCG_surface_init(gdi, surface) */
    FCG_Surface_Initialize(surface, machine, gdi);

    return FCG_SUCCESS;
}