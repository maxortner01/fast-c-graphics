#include <fcg/fcg.h>

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
    FCG_Result result = FCG_CreateRenderEnvironment(&surface, &machine, &gdi, FCG_SURFACE_WINDOW, &data);
    if (result)
    {
        printf("Surface failed to create! Error: %i\n", result);
        return 1;
    }
    printf("Context size: %u x %u.\n", surface.size.width, surface.size.height);
    printf("Graphics Device: %s.\n", machine.graphics_devices->name);
    //printf("Graphics queue:  %i.\n", machine.graphics_devices->graphics_queue);
    //printf("Required Device Extensions (%i): %s.\n", machine.graphics_devices->required_extensions_count, machine.graphics_devices->required_extensions[0]);

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
    FCG_DestroySurface(&surface);

    return 0;
}