#include <fcg/graphics/device.h>
#include "../assert.c"


FCG_Result 
free_graphics_device(
    FCG_GraphicsDevice* FCG_CR device)
{
    free(device->name);
    
    for (U32 i = 0; i < device->required_extensions_count; i++)
        free(device->required_extensions[i]);
    free(device->required_extensions);

    return FCG_SUCCESS;
}

FCG_Result 
FCG_DestroyMachine(
    FCG_Machine* FCG_CR machine)
{
    FCG_assert(machine->active);

    while (machine->destructor_stack.object_count)
    {
        FCG_DestructorElement element;
        FCG_Memory_Pop(&machine->destructor_stack, &element);
        element.handle(&element.arguments);
        FCG_Memory_DestroyStack(&element.arguments);
    }
    FCG_Memory_DestroyStack(&machine->destructor_stack);

    /* Free the graphics_device */
    for (uint32_t i = 0; i < machine->graphics_device_count; i++)
    {
        FCG_Result result = free_graphics_device(machine->graphics_devices + i);
        if (result) return result;
    }

    if (machine->graphics_devices) free(machine->graphics_devices);
    memset(machine, 0, sizeof(FCG_Machine));

    return FCG_SUCCESS;
}