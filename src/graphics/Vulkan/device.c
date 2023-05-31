#include <fcg/graphics/device.h>
#include "../../assert.c"

#include <vulkan/vulkan.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

/* Vulkan object destructors */

// Stack: |       --- top ---          |
//        | FCG_Handle vulkan_instance |
void 
destroy_instance(
    FCG_Memory_Stack* FCG_CR stack)
{
    FCG_Handle handle;
    FCG_Memory_Pop(stack, &handle);
    
    vkDestroyInstance(handle, NULL);
}

// Stack: |       --- top ---          |
//        | FCG_Handle vulkan_instance |
//        | FCG_Handle surface         |
void 
destroy_surface(
    FCG_Memory_Stack* FCG_CR stack)
{
    FCG_Handle instance; FCG_Handle surface;
    FCG_Memory_Pop(stack, &instance);
    FCG_Memory_Pop(stack, &surface);

    vkDestroySurfaceKHR(instance, surface, NULL);
}

VkResult
create_vulkan_instance(
    VkInstance* instance)
{
    /* Get the extensions */
    U32 present_index = 0;
    U32 sdl_extension_count;
    FCG_assert(SDL_Vulkan_GetInstanceExtensions(&sdl_extension_count, NULL));

#ifdef __APPLE__
    U32 extra_indices = 2;
#else
    U32 extra_indices = 0;
#endif

#ifdef DEBUG
    extra_indices += 1;
#endif

    const char** extensions = malloc(sizeof(void*) * (sdl_extension_count + extra_indices));
    SDL_Vulkan_GetInstanceExtensions(&sdl_extension_count, extensions);

#ifdef __APPLE__
    *(extensions + sdl_extension_count + present_index++) = VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME;
    *(extensions + sdl_extension_count + present_index++) = VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME;
#endif

#ifdef DEBUG
    *(extensions + sdl_extension_count + present_index++) = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;

    const char* validation_layer = "VK_LAYER_KHRONOS_validation";
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

#ifdef DEBUG
    create_info.enabledLayerCount = 1;
    create_info.ppEnabledLayerNames = &validation_layer;
#endif

    VkResult result = vkCreateInstance(&create_info, NULL, instance);

    free(extensions);
    return result;
}

#ifdef DEBUG

VKAPI_ATTR VkBool32 VKAPI_CALL 
debug_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData)
{
    switch (messageSeverity)
    {
    default: break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
        printf("Info: %s\n", pCallbackData->pMessage); break;
    
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
        printf("Warning: %s\n", pCallbackData->pMessage); break;
    
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
        printf("Error: %s\n", pCallbackData->pMessage); break;
    }

    return VK_FALSE;
}

VkResult
_CreateDebugUtilsMessengerEXT(
    VkInstance instance, 
    const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, 
    const VkAllocationCallbacks* pAllocator, 
    VkDebugUtilsMessengerEXT* pDebugMessenger)
{
    PFN_vkCreateDebugUtilsMessengerEXT func = vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func) return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    return VK_ERROR_EXTENSION_NOT_PRESENT;
}

void 
_DestroyDebugUtilsMessengerEXT(
    VkInstance instance, 
    VkDebugUtilsMessengerEXT debugMessenger, 
    const VkAllocationCallbacks* pAllocator) 
{
    PFN_vkDestroyDebugUtilsMessengerEXT func = vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func) func(instance, debugMessenger, pAllocator);
}

void destroy_debug_messenger(
    FCG_Memory_Stack* FCG_CR stack)
{
    FCG_Handle instance; FCG_Handle messenger;
    FCG_Memory_Pop(stack, &instance);
    FCG_Memory_Pop(stack, &messenger);

    _DestroyDebugUtilsMessengerEXT(instance, messenger, NULL);
}

VkResult 
create_debug_messenger(
    VkInstance                FCG_CR instance,
    VkDebugUtilsMessengerEXT* FCG_CR messenger)
{
    printf("Enabling debug messenger.\n");

    VkDebugUtilsMessengerCreateInfoEXT create_info = {
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
        .pNext = NULL,
        .flags = 0,
        .messageSeverity = 0,
        .messageType = 0,
        .pfnUserCallback = debug_callback,
        .pUserData = NULL
    };

    create_info.messageSeverity |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;
    create_info.messageSeverity |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
    create_info.messageSeverity |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

    create_info.messageType |= VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT;
    create_info.messageType |= VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
    create_info.messageType |= VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

    return _CreateDebugUtilsMessengerEXT(instance, &create_info, NULL, messenger);
}

#endif

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

#ifdef DEBUG
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

    /* Create surface */
    VkSurfaceKHR vulkan_surface;
    FCG_assert(SDL_Vulkan_CreateSurface(surface->handle, machine->handle, &vulkan_surface)); // Implement destructor stack to handle this
    surface->context = vulkan_surface;

    /* Handle surface destruction */
    {
        FCG_DestructorElement element = {
            .handle    = destroy_surface
        };

        FCG_Memory_InitializeStack(&element.arguments);
        FCG_Memory_PushStack(&element.arguments, &surface->context, sizeof(FCG_Handle));
        FCG_Memory_PushStack(&element.arguments, &machine->handle,  sizeof(FCG_Handle));

        FCG_Memory_PushStack(&machine->destructor_stack, &element, sizeof(FCG_DestructorElement));
    }

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
        FCG_DestructorElement element;
        FCG_Memory_Pop(&machine->destructor_stack, &element);
        element.handle(&element.arguments);
        FCG_Memory_DestroyStack(&element.arguments);
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
