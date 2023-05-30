#include <fcg/graphics/device.h>
#include "../../assert.c"

#include <vulkan/vulkan.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

VkInstance
create_vulkan_instance()
{
    /* Get the extensions */
    uint32_t present_index = 0;
    uint32_t sdl_extension_count;
    FCG_assert(SDL_Vulkan_GetInstanceExtensions(&sdl_extension_count, NULL));

#ifdef __APPLE__
    uint32_t extra_indices = 2;
#else
    uint32_t extra_indices = 0;
#endif

    const char** extensions = malloc(sdl_extension_count + extra_indices);
    SDL_Vulkan_GetInstanceExtensions(&sdl_extension_count, extensions);

#ifdef __APPLE__
    *(extensions + sdl_extension_count + present_index++) = VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME;
    *(extensions + sdl_extension_count + present_index++) = VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME;
#endif

    /* Set up the info */
    VkApplicationInfo app_info = {
        .sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName   = "fcg",
        .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
        .pEngineName        = "No Engine",
        .engineVersion      = VK_MAKE_VERSION(1, 0, 0),
        .apiVersion         = VK_API_VERSION_1_0
    };

    VkInstanceCreateInfo create_info = {
        .sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo        = &app_info,
        .enabledExtensionCount   = sdl_extension_count + extra_indices,
        .ppEnabledExtensionNames = extensions,
        .enabledLayerCount       = 0
    };

#ifdef __APPLE__
    // Because apple is special
    create_info.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#endif

    VkInstance instance = NULL;
    VkResult result = vkCreateInstance(&create_info, NULL, &instance);
    
    free(extensions);
    return instance;
}

FCG_Result FCG_InitializeMachine(
    FCG_Machine* FCG_CR machine,
    const FCG_Surface* FCG_CR surface)
{
    VkInstance instance = create_vulkan_instance();
    if (!instance) return FCG_INSTANCE_CREATE_FAILED;

    /* Reset the memory */
    memset(machine, 0, sizeof(FCG_Machine));
    machine->active = FCG_True;
    machine->handle = instance;

    /* Set the graphics device as the default OpenGL one */
    machine->graphics_device_count = 1;
    machine->graphics_devices = malloc(sizeof(FCG_GraphicsDevice));

    //FCG_GraphicsDevice* graphics_device = machine->graphics_devices;
    //graphics_device->name = glGetString(GL_RENDERER);

    return FCG_SUCCESS;
}

FCG_Result FCG_DestroyMachine(FCG_Machine* FCG_CR machine)
{
    FCG_assert(machine->active);

    vkDestroyInstance(machine->handle, NULL);

    if (machine->graphics_devices) free(machine->graphics_devices);
    memset(machine, 0, sizeof(FCG_Machine));

    return FCG_SUCCESS;
}
