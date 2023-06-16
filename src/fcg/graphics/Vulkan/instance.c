#include "instance.h"

/* Vulkan object destructors */

// Stack: |       --- top ---          |
//        | FCG_Handle vulkan_instance |
void 
destroy_instance(
    FCG_Memory_Stack* FCG_CR stack)
{
    printf("destroying instance\n");

    FCG_Handle handle;
    FCG_Memory_PopStack(stack, &handle);
    
    vkDestroyInstance(handle, NULL);
}

// Stack: |       --- top ---          |
//        | FCG_Handle vulkan_instance |
//        | FCG_Handle surface         |
void 
destroy_surface(
    FCG_Memory_Stack* FCG_CR stack)
{
    printf("destroying surface\n");

    FCG_Handle instance; FCG_Handle surface;
    FCG_Memory_PopStack(stack, &instance);
    FCG_Memory_PopStack(stack, &surface);

    vkDestroySurfaceKHR(instance, surface, NULL);
}

#ifdef FCG_DEBUG

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

void set_debug_create_info(VkDebugUtilsMessengerCreateInfoEXT* create_info)
{
    memset(create_info, 0, sizeof(VkDebugUtilsMessengerCreateInfoEXT));
    create_info->sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    create_info->pNext = NULL;
    create_info->flags = 0;
    create_info->messageSeverity = 0;
    create_info->messageType = 0;
    create_info->pfnUserCallback = debug_callback;
    create_info->pUserData = NULL;

    create_info->messageSeverity |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;
    create_info->messageSeverity |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
    create_info->messageSeverity |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

    create_info->messageType |= VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT;
    create_info->messageType |= VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
    create_info->messageType |= VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
}

#endif 

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

#ifdef FCG_DEBUG
    extra_indices += 1;
#endif

    const char** extensions = malloc(sizeof(void*) * (sdl_extension_count + extra_indices));
    SDL_Vulkan_GetInstanceExtensions(&sdl_extension_count, extensions);

#ifdef __APPLE__
    *(extensions + sdl_extension_count + present_index++) = VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME;
    *(extensions + sdl_extension_count + present_index++) = VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME;
#endif

#ifdef FCG_DEBUG
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

#ifdef FCG_DEBUG
    create_info.enabledLayerCount = 1;
    create_info.ppEnabledLayerNames = &validation_layer;

    VkDebugUtilsMessengerCreateInfoEXT debug_info;
    set_debug_create_info(&debug_info);
    create_info.pNext = &debug_info;
#endif

    VkResult result = vkCreateInstance(&create_info, NULL, instance);

    free(extensions);
    return result;
}

#ifdef FCG_DEBUG

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
    printf("destroying debug messenger\n");

    FCG_Handle instance; FCG_Handle messenger;
    FCG_Memory_PopStack(stack, &instance);
    FCG_Memory_PopStack(stack, &messenger);

    _DestroyDebugUtilsMessengerEXT(instance, messenger, NULL);
}

VkResult 
create_debug_messenger(
    VkInstance                FCG_CR instance,
    VkDebugUtilsMessengerEXT* FCG_CR messenger)
{
    printf("Enabling debug messenger.\n");

    VkDebugUtilsMessengerCreateInfoEXT create_info;
    set_debug_create_info(&create_info);

    return _CreateDebugUtilsMessengerEXT(instance, &create_info, NULL, messenger);
}

#endif