#include <fcg/graphics/device.h>
#include "../../assert.c"

#include <GL/glew.h>
#include <SDL3/SDL.h>

static FCG_Bool GL_Initialized = FCG_False;

void initialize_GL()
{
    glewExperimental = GL_TRUE;
    FCG_assert(glewInit() == GLEW_OK);

    GL_Initialized = FCG_True;
}

FCG_Result FCG_InitializeMachine(
    FCG_Machine* FCG_CR machine,
    const FCG_Surface* FCG_CR surface)
{
    /* Create the OpenGL context */
    SDL_GLContext context = SDL_GL_CreateContext((SDL_Window*)surface->handle);
    if (context == NULL) 
    {
        printf("%s\n", SDL_GetError());
        return FCG_CONTEXT_CREATE_FAILED;
    }
    
    /* Here's where we initialize GLEW */
    if (!GL_Initialized) initialize_GL();

    /* Reset the memory */
    memset(machine, 0, sizeof(FCG_Machine));
    machine->active = FCG_True;

    /* Set the graphics device as the default OpenGL one */
    machine->graphics_device_count = 1;
    machine->graphics_devices = malloc(sizeof(FCG_GraphicsDevice));

    FCG_GraphicsDevice* graphics_device = machine->graphics_devices;
    graphics_device->name = glGetString(GL_RENDERER);

    return FCG_SUCCESS;
}

FCG_Result FCG_DestroyMachine(FCG_Machine* FCG_CR machine)
{
    FCG_assert(machine->active);

    if (machine->graphics_devices)  free(machine->graphics_devices);
    memset(machine, 0, sizeof(FCG_Machine));

    return FCG_SUCCESS;
}