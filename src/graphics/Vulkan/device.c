#include <fcg/graphics/device.h>
#include "../../assert.c"

#include <vulkan/vulkan.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

#include "instance.h"

FCG_Result FCG_InitializeMachine(
    FCG_Machine* FCG_CR machine)
{
    VkInstance        instance;
    VkPhysicalDevice* devices;

    /* Create a dummy surface */
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow("dummy", 1, 1, SDL_WINDOW_VULKAN);

    /* Initialize vulkan */
    VkResult result = create_vulkan_instance(&instance);
    if (result != VK_SUCCESS) return FCG_INSTANCE_CREATE_FAILED;

    /* Initialize the given machine structure */
    memset(machine, 0, sizeof(FCG_Machine));
    machine->active = FCG_True;
    machine->handle = instance;

    FCG_Memory_InitializeStack(&machine->destructor_stack);

    /* Handle instance destruction */
    {
        FCG_DestructorElement element = {
            .handle    = destroy_instance
        };

        // This gets destroyed when the handle function is called
        FCG_Memory_InitializeStack(&element.arguments);
        FCG_Memory_PushStack(&element.arguments, &machine->handle, sizeof(FCG_Handle));

        FCG_Memory_PushStack(&machine->destructor_stack, &element, sizeof(FCG_DestructorElement));
    }

#ifdef FCG_DEBUG
    /* Create debug messenger */
    VkDebugUtilsMessengerEXT messenger;
    result = create_debug_messenger(machine->handle, &messenger);
    if (result != VK_SUCCESS) return FCG_DEBUG_ERROR;

    machine->debug = messenger;

    /* Handle debug messenger destruction */
    {
        FCG_DestructorElement element = {
            .handle = destroy_debug_messenger
        };

        FCG_Memory_InitializeStack(&element.arguments);
        FCG_Memory_PushStack(&element.arguments, &machine->debug,  sizeof(FCG_Handle));
        FCG_Memory_PushStack(&element.arguments, &machine->handle, sizeof(FCG_Handle));

        FCG_Memory_PushStack(&machine->destructor_stack, &element, sizeof(FCG_DestructorElement));
    }
#endif

    /* Create dummy surface */
    VkSurfaceKHR vulkan_surface;
    FCG_assert(SDL_Vulkan_CreateSurface(window, machine->handle, &vulkan_surface)); 

    /* Now we'll grab the graphics devices */
    vkEnumeratePhysicalDevices(machine->handle, &machine->graphics_device_count, NULL);

    // Freed before return
    devices = calloc(machine->graphics_device_count, sizeof(VkPhysicalDevice));
    vkEnumeratePhysicalDevices(machine->handle, &machine->graphics_device_count, devices);

    // Freed in FCG_DestroyMachine
    machine->graphics_devices = calloc(machine->graphics_device_count, sizeof(FCG_GraphicsDevice));

    /* Go through the devices and get the properties */
    for (uint32_t i = 0; i < machine->graphics_device_count; i++)
    {
        uint32_t name_length, queue_count;
        VkPhysicalDeviceProperties properties;
        VkQueueFamilyProperties*   queue_families;
        const VkPhysicalDevice physical_device = *(devices + i);

        vkGetPhysicalDeviceProperties(physical_device, &properties);
        
        FCG_GraphicsDevice* device = machine->graphics_devices + i;
        memset(device, 0, sizeof(FCG_GraphicsDevice));
        device->handle = physical_device;
        
        /* Grab the name */
        name_length = strlen(properties.deviceName) + 1;
        device->name = malloc(name_length); // Freed in FCG_DestroyGraphicsDevice
        memcpy(device->name, &properties.deviceName[0], name_length);

        /* Determine the queues */
        vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_count, NULL);
        queue_families = calloc(queue_count, sizeof(VkQueueFamilyProperties)); // Freed in loop
        vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_count, queue_families);

        /* Initialize the values */
        device->graphics_queue = -1;
        device->present_queue  = -1;
        for (U32 j = 0; j < queue_count; j++)
        {
            if (queue_families[j].queueFlags & VK_QUEUE_GRAPHICS_BIT) device->graphics_queue = j;

            VkBool32 present_support = 0;
            vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, j, vulkan_surface, &present_support);
            if (present_support) device->present_queue = j;
        }

        /* Determine the required extensions 
        switch (surface->type)
        {
        case FCG_SURFACE_WINDOW:
        {
            const char* required_extension = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
            U32 ext_name_length = strlen(required_extension) + 1;

            device->required_extensions_count = 1;
            device->required_extensions = calloc(device->required_extensions_count, sizeof(void*)); // freed in FCG_DestroyGraphicsDevice
            device->required_extensions[0] = calloc(ext_name_length, 1); // freed in FCG_DestroyGraphicsDevice
            memcpy(device->required_extensions[0], &required_extension[0], ext_name_length);
            break;
        }
        default: break;
        }*/

        free(queue_families);
    }

    vkDestroySurfaceKHR(machine->handle, vulkan_surface, NULL);
    SDL_DestroyWindow(window);

    free(devices);
    return FCG_SUCCESS;
}
