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
    result = FCG_CreateSurface(surface, type, data);
    if (result) return result;

    result = FCG_InitializeMachine(machine, surface);
    if (result) return result;

    result = FCG_CreateGraphicsInstance(gdi, surface, machine);
    if (result) return result;

    return FCG_SUCCESS;
}