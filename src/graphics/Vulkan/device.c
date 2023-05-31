#include <fcg/graphics/device.h>
#include "../../assert.c"

#include <vulkan/vulkan.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

VkResult
create_vulkan_instance(
    VkInstance* instance)
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

    const char** extensions = malloc(sizeof(void*) * (sdl_extension_count + extra_indices));
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
        .apiVersion         = VK_API_VERSION_1_0,
        .pNext              = NULL
    };

    VkInstanceCreateInfo create_info = {
        .sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo        = &app_info,
        .enabledExtensionCount   = sdl_extension_count + extra_indices,
        .ppEnabledExtensionNames = extensions,
        .enabledLayerCount       = 0,
        .ppEnabledLayerNames     = NULL,
#   ifdef __APPLE__
        .flags                   = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR,
#   else
        .flags                   = 0,
#endif
        .pNext                   = NULL
    };

    VkResult result = vkCreateInstance(&create_info, NULL, instance);

    free(extensions);
    return result;
}

void test()
{
    printf("test\n");
}

typedef void (*DESTRUCTOR)(void);

FCG_Result FCG_InitializeMachine(
    FCG_Machine* FCG_CR machine,
    FCG_Surface* FCG_CR surface)
{
    VkInstance        instance;
    VkPhysicalDevice* devices;

    /* Initialize vulkan */
    VkResult result = create_vulkan_instance(&instance);
    if (result != VK_SUCCESS) return FCG_INSTANCE_CREATE_FAILED;

    /* Initialize the given machine structure */
    memset(machine, 0, sizeof(FCG_Machine));
    machine->active = FCG_True;
    machine->handle = instance;

    FCG_Memory_InitializeStack(&machine->destructor_stack);

    /* As easy as this! */
    //DESTRUCTOR dest = test;
    //FCG_Memory_PushStack(&machine->destructor_stack, &dest, sizeof(DESTRUCTOR));

    /* Create surface */
    VkSurfaceKHR vulkan_surface;
    FCG_assert(SDL_Vulkan_CreateSurface(surface->handle, machine->handle, &vulkan_surface)); // Implement destructor stack to handle this
    surface->context = vulkan_surface;

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

            VkBool32 present_support;
            vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, j, vulkan_surface, &present_support);
            if (present_support) device->present_queue = j;
        }

        /* Determine the required extensions */
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
        }

        free(queue_families);
    }

    free(devices);
    return FCG_SUCCESS;
}

FCG_Result FCG_DestroyGraphicsDevice(FCG_GraphicsDevice* FCG_CR device)
{
    free(device->name);
    
    for (U32 i = 0; i < device->required_extensions_count; i++)
        free(device->required_extensions[i]);
    free(device->required_extensions);

    return FCG_SUCCESS;
}

FCG_Result FCG_DestroyMachine(FCG_Machine* FCG_CR machine)
{
    FCG_assert(machine->active);

    while (machine->destructor_stack.object_count)
    {
        DESTRUCTOR dest;
        FCG_Memory_Pop(&machine->destructor_stack, &dest);
        dest();
    }

    /* Free the graphics_device */
    for (uint32_t i = 0; i < machine->graphics_device_count; i++)
    {
        FCG_Result result = FCG_DestroyGraphicsDevice(machine->graphics_devices + i);
        if (result) return result;
    }

    if (machine->graphics_devices) free(machine->graphics_devices);
    memset(machine, 0, sizeof(FCG_Machine));

    vkDestroyInstance(machine->handle, NULL);

    FCG_Memory_DestroyStack(&machine->destructor_stack);

    return FCG_SUCCESS;
}
