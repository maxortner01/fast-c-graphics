#include <fcg/graphics/surface.h>
#include "../assert.c"

#include <SDL3/SDL.h>

#define FCG_CONTEXT_TYPE_NOT_VALID FCG_False

static FCG_Bool SDL_Initialized = FCG_False;

void initialize_SDL()
{
    FCG_assert(!SDL_Init(SDL_INIT_VIDEO));
    SDL_Initialized = FCG_True;
}

FCG_Result create_window(FCG_Surface* FCG_CR surface, FCG_WindowData* FCG_CR data)
{
    /* Copy over relevant information into surface and create the window with SDL */
    surface->type = FCG_SURFACE_WINDOW;

    uint32_t flags;
#if FCG_RENDER_API_TYPE == FCG_OPENGL
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 4 );
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 1 );
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE );
    flags = SDL_WINDOW_OPENGL;
#elif FCG_RENDER_API_TYPE == FCG_VULKAN
    flags = SDL_WINDOW_VULKAN;
#else
#   error "Render API not supported."
#endif

    surface->handle = (FCG_Handle*)SDL_CreateWindow(data->title, data->size.width, data->size.height, flags);
    memcpy(&surface->size, &data->size, sizeof(FCG_ContextSize));

    /* Pass along SDL error info */
    if (surface->handle == NULL) return FCG_WINDOW_CREATE_FAILED;

    return FCG_SUCCESS;
}


FCG_Result FCG_Surface_Create(FCG_Surface* FCG_CR surface, FCG_ContextType type, FCG_DataHandle const data)
{
    /* If we create a surface, we need to at some point initialize SDL */
    if (!SDL_Initialized) initialize_SDL();

    surface->active = FCG_True;
    FCG_Memory_InitializeStack(&surface->destructor_stack);

    switch (type)
    {
        case FCG_SURFACE_WINDOW: return create_window(surface, (FCG_WindowData*)data);
        default: FCG_assert(FCG_CONTEXT_TYPE_NOT_VALID);
    }
}

void 
FCG_Surface_Destroy(
    FCG_Memory_Stack* FCG_CR stack)
{
    FCG_Surface* surface;
    FCG_Memory_Pop(stack, &surface);

    /* Only active surfaces can be destroyed (to prevent 'double free') */
    FCG_assert(surface->active);
    surface->active = FCG_False;

    // Free the surface image list
    while (surface->destructor_stack.object_count)
    {
        FCG_DestructorElement element;
        FCG_Memory_Pop(&surface->destructor_stack, &element);
        element.handle(&element.arguments);
        FCG_Memory_DestroyStack(&element.arguments);
    }
    FCG_Memory_DestroyStack(&surface->destructor_stack);

    if (surface->surface_image.images) free(surface->surface_image.images);

    switch (surface->type)
    {
    case FCG_SURFACE_WINDOW: 
    {
        SDL_DestroyWindow(surface->handle);
        if (SDL_Initialized) SDL_Quit();
        return;
    }
    default: FCG_assert(FCG_CONTEXT_TYPE_NOT_VALID);
    }
}

void FCG_DisplaySurface(const FCG_Surface* FCG_CR surface)
{
    FCG_assert(surface->active);

    switch (surface->type)
    {
    case FCG_SURFACE_WINDOW: SDL_UpdateWindowSurface((SDL_Window*)surface->handle); return;
    default: FCG_assert(FCG_CONTEXT_TYPE_NOT_VALID);
    }
}

void FCG_KeepSurfaceOpen(const FCG_Surface* FCG_CR surface)
{
    SDL_Event e;
    FCG_Bool quit = FCG_False;
    while (!quit)
    {
        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_EVENT_QUIT) quit = FCG_True;
        }
    }
}

/*
FCG_WindowFlags FCG_CreateWindow(FCG_Window* window)
{
    FCG_assert(window);

    if (SDL_Init(SDL_INIT_VIDEO) < 0)
        return FCG_INIT_FAILED;

    window->handle = SDL_CreateWindow("test", 1280, 720, 0);
    if (window->handle == NULL)
        return FCG_WINDOW_CREATE_FAILED;

    return FCG_SUCCESS;
}

void FCG_DestroyWindow(FCG_Window* window)
{
    FCG_assert(window);

    SDL_DestroyWindow(window->handle);
    SDL_Quit();
    window->handle = NULL;
}*/

#undef FCG_CONTEXT_TYPE_NOT_VALID