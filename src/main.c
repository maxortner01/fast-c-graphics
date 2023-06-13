#include <fcg/fcg.h>
#include "assert.c"
#include <stdio.h>

int main(int argc, char** argv)
{
    FCG_WindowData data = {
        .size = {
            .width = 1280,
            .height = 720
        },
        .title  = "hello"
    };

    FCG_Surface surface;
    FCG_Machine machine;
    FCG_GDI gdi;

    FCG_assert( 
        !FCG_InitializeMachine(&machine) &&
        !FCG_Surface_Create(&surface, FCG_SURFACE_WINDOW, &data) &&
        !FCG_CreateGraphicsInstance(&gdi, &surface, &machine) 
    );

    printf("Context size: %u x %u.\n", surface.size.width, surface.size.height);
    printf("Graphics Device: %s.\n", machine.graphics_devices->name);
    //printf("Graphics queue:  %i.\n", machine.graphics_devices->graphics_queue);
    //printf("Required Device Extensions (%i): %s.\n", machine.graphics_devices->required_extensions_count, machine.graphics_devices->required_extensions[0]);

    FCG_Transformation_Layout layout;
    FCG_Transformation_InitLayout(&layout);
    layout.terminal = &surface;

    FCG_Module_Programmable shaders[2];
    shaders[0].handle = NULL;
    shaders[0].type = FCG_SHADER_VERTEX;
    shaders[0].filename = "../shaders/vert.spv";
    shaders[0].file_type = FCG_SHADER_FILE_TYPE_SPRV;
    shaders[0].device = &gdi.rendering_devices[0];
    
    shaders[1].handle = NULL;
    shaders[1].type = FCG_SHADER_FRAGMENT;
    shaders[1].filename = "../shaders/frag.spv";
    shaders[1].file_type = FCG_SHADER_FILE_TYPE_SPRV;
    shaders[1].device = &gdi.rendering_devices[0];

    FCG_Module_Transformation pipeline;
    FCG_Module_PipelineInfo pipeline_info = {
        .shader_count = 2,
        .shaders      = &shaders[0]
    };

    //FCG_assert(FCG_Module_ConstructPipeline(&pipeline, &pipeline_info, &surface) == FCG_SUCCESS);
    FCG_Transformation_Push(&layout, &pipeline);

    FCG_DisplaySurface(&surface);

    /*
    SDL_Surface* surface = SDL_GetWindowSurface(window.handle);
    SDL_FillSurfaceRect(surface, NULL, SDL_MapRGB(surface->format, 0xFF, 0xFF, 0xFF));
    SDL_UpdateWindowSurface(window.handle);

    SDL_Event e;
    int8_t quit = 0;
    while (!quit)
    {
        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_EVENT_QUIT) quit = 1;
        }
    }*/

    FCG_KeepSurfaceOpen(&surface);

    FCG_DestroyGraphicsInstance(&gdi);
    FCG_DestroyMachine(&machine);

    return 0;
}