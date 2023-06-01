#include <fcg/graphics/gdi.h>
#include "../../assert.c"

#include <vulkan/vulkan.h>

#define INITIALIZE(type, name) type name; memset(&name, 0, sizeof(type))

void 
destroy_rendering_device(
    FCG_Memory_Stack* FCG_CR stack)
{
    FCG_RenderingDevice device;
    FCG_Memory_Pop(stack, &device);

    FCG_assert(device.active);

    vkDestroyDevice(device.handle, NULL);
}

VkResult 
create_logic_device(
    FCG_RenderingDevice* FCG_CR rendering_device,
    const   FCG_Machine* FCG_CR machine)
{
    /* Pick out the best device */
    int32_t index = 0;
    for (uint32_t i = 0; i < machine->graphics_device_count; i++)
    {
        /* Prefer the discrete card */
        if (machine->graphics_devices->discrete)
        { index = i; break; }
    }

    const FCG_GraphicsDevice* suitable_device = machine->graphics_devices + index;

    /* Deal with extensions */
    U32 present_index = 0;
#ifdef __APPLE__
    CU32 extra_extensions = 1;
#else
    CU32 extra_extensions = 0;
#endif
    
    CU32 required_count   = suitable_device->required_extensions_count;
    CU32 total_extensions = required_count + extra_extensions;
    char** extensions = calloc(total_extensions, sizeof(char*)); // freed at return
    memcpy(extensions, suitable_device->required_extensions, sizeof(char*) * required_count);
    
#ifdef __APPLE__
    // Load in the extra extension needed for apple devices
    const char* portability_extension = "VK_KHR_portability_subset";
    CU32 extension_name_len = strlen(portability_extension) + 1;
    extensions[required_count + present_index] = calloc(extension_name_len, 1); // freed at return
    memcpy(extensions[required_count + present_index++], &portability_extension[0], extension_name_len);
#endif

    //for (U32 i = 0; i < total_extensions; i++) printf("Extension: %s\n", extensions[i]);

    /* We require a graphics queue at the very least */
    FCG_assert(suitable_device->graphics_queue >= 0);
    FCG_assert(suitable_device->present_queue >= 0);

    const float priorities = 1.f;
    INITIALIZE(VkPhysicalDeviceFeatures, features);

    U32 queue_count = 1;
    if (suitable_device->graphics_queue != suitable_device->present_queue) queue_count++;
    VkDeviceQueueCreateInfo* queue_create_infos = calloc(queue_count, sizeof(VkDeviceQueueCreateInfo)); // freed before return

    queue_create_infos->sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_create_infos->pNext            = NULL;
    queue_create_infos->pQueuePriorities = &priorities;
    queue_create_infos->queueCount       = 1;
    queue_create_infos->queueFamilyIndex = (uint32_t)suitable_device->graphics_queue;
    queue_create_infos->flags            = 0;

    if (queue_count == 2)
    {
        (queue_create_infos + 1)->sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        (queue_create_infos + 1)->pNext            = NULL;
        (queue_create_infos + 1)->pQueuePriorities = &priorities;
        (queue_create_infos + 1)->queueCount       = 1;
        (queue_create_infos + 1)->queueFamilyIndex = (uint32_t)suitable_device->present_queue;
        (queue_create_infos + 1)->flags            = 0;
    }

    VkDeviceCreateInfo create_info = {
        .sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext                   = NULL,
        .flags                   = 0,
        .queueCreateInfoCount    = queue_count,
        .pQueueCreateInfos       = queue_create_infos,
        .enabledExtensionCount   = total_extensions,
        .ppEnabledExtensionNames = extensions,
        .enabledLayerCount       = 0,
        .ppEnabledLayerNames     = NULL,
        .pEnabledFeatures        = &features
    };

    VkDevice logic_device;
    VkResult result = vkCreateDevice(suitable_device->handle, &create_info, NULL, &logic_device);
    if (result == VK_SUCCESS) rendering_device->active = FCG_True;

    rendering_device->handle = logic_device;
    rendering_device->suitable_device = suitable_device;

    for (U32 i = 0; i < extra_extensions; i++)
        free(extensions[required_count + i]);
    free(extensions);
    free(queue_create_infos);
    
    return result;
}

VkSurfaceFormatKHR
choose_swapchain_format(
    VkSurfaceFormatKHR* FCG_CR formats,
    CU32                       format_count)
{
    for (U32 i = 0; i < format_count; i++)
        if (formats[i].format == VK_FORMAT_B8G8R8A8_SRGB && formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            return formats[i];

    return formats[0];
}

VkPresentModeKHR
choose_swapchain_present_mode(
    VkPresentModeKHR* FCG_CR present_modes, 
    CU32                     present_mode_count)
{
    for (U32 i = 0; i < present_mode_count; i++)
        if (present_modes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
            return VK_PRESENT_MODE_MAILBOX_KHR;
    
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D
choose_swapchain_extent(
    VkSurfaceCapabilitiesKHR capabilities,
    FCG_Surface* FCG_CR      surface)
{
    if (capabilities.currentExtent.width != UINT32_MAX)
        return capabilities.currentExtent;

    VkExtent2D extent;

    extent.width  = surface->size.width;
    extent.height = surface->size.height;
    if (extent.width > capabilities.maxImageExtent.width)
        extent.width = capabilities.maxImageExtent.width;
    if (extent.height > capabilities.maxImageExtent.height)
        extent.height = capabilities.maxImageExtent.height; 

    return extent;
}

VkResult 
create_swap_chain(
    FCG_GDI*     FCG_CR gdi,
    FCG_Surface* FCG_CR surface)
{
    const VkSurfaceKHR         const vk_surface      = surface->context;
    const FCG_RenderingDevice* const render_device   = gdi->rendering_devices;
    const FCG_GraphicsDevice*  const graphics_device = render_device->suitable_device;
    const VkPhysicalDevice     const physical_device = render_device->suitable_device->handle;

    VkSurfaceCapabilitiesKHR capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, vk_surface, &capabilities);

    U32 format_count; U32 present_mode_count;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, vk_surface, &format_count, NULL);
    vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, vk_surface, &present_mode_count, NULL);

    VkSurfaceFormatKHR* formats       = calloc(format_count, sizeof(VkSurfaceFormatKHR));
    VkPresentModeKHR*   present_modes = calloc(present_mode_count, sizeof(VkPresentModeKHR));
    vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, vk_surface, &format_count, formats);
    vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, vk_surface, &present_mode_count, present_modes);

    VkSurfaceFormatKHR surface_format = choose_swapchain_format(formats, format_count);
    VkPresentModeKHR   present_mode   = choose_swapchain_present_mode(present_modes, present_mode_count);
    VkExtent2D         extent         = choose_swapchain_extent(capabilities, surface);

    U32 image_count = capabilities.minImageCount + 1;
    if (image_count > capabilities.maxImageCount) image_count = capabilities.maxImageCount;

    VkSwapchainCreateInfoKHR create_info = {
        .sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface          = vk_surface,
        .minImageCount    = image_count,
        .imageFormat      = surface_format.format,
        .imageColorSpace  = surface_format.colorSpace,
        .imageExtent      = extent,
        .imageArrayLayers = 1,
        .imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .preTransform     = capabilities.currentTransform,
        .compositeAlpha   = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode      = present_mode,
        .clipped          = VK_TRUE,
        .oldSwapchain     = VK_NULL_HANDLE,
    };

    FCG_assert(graphics_device->present_queue >= 0);
    FCG_assert(graphics_device->graphics_queue >= 0);

    CU32 queue_family_indices[] = {
        graphics_device->graphics_queue, graphics_device->present_queue
    };

    if (queue_family_indices[0] != queue_family_indices[1])
    {
        create_info.imageSharingMode      = VK_SHARING_MODE_CONCURRENT;
        create_info.queueFamilyIndexCount = 2;
        create_info.pQueueFamilyIndices   = queue_family_indices;
    }
    else
    {
        create_info.imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE;
        create_info.queueFamilyIndexCount = 0;
        create_info.pQueueFamilyIndices   = NULL;
    }

    VkSwapchainKHR swapchain;
    VkResult result = vkCreateSwapchainKHR(render_device->handle, &create_info, NULL, &swapchain);

    surface->surface_image.handle = swapchain;

    free(formats); free(present_modes);
    return result;
}

void 
destroy_swapchain(
    FCG_Memory_Stack* FCG_CR stack)
{
    FCG_Handle device; FCG_Handle swapchain;
    FCG_Memory_Pop(stack, &device);
    FCG_Memory_Pop(stack, &swapchain);

    vkDestroySwapchainKHR(device, swapchain, NULL);
}

FCG_Result 
FCG_CreateGraphicsInstance(
    FCG_GDI* FCG_CR instance,  
    FCG_Surface* FCG_CR surface,
    FCG_Machine* FCG_CR machine)
{
    memset(instance, 0, sizeof(FCG_GDI));
    instance->active = FCG_True;

    // create rendering device from the physical devices given in the machine
    // Create logic device
    instance->rendering_device_count = 1;
    instance->rendering_devices = calloc(1, sizeof(FCG_RenderingDevice)); // freed in FCG_DestroyGraphicsInstance
    VkResult result = create_logic_device(instance->rendering_devices, machine);
    if (result != VK_SUCCESS)
    {
        printf("Vulkan error: %i\n", result);
        return FCG_RENDERING_DEVICE_FAILED;
    }

    {
        FCG_DestructorElement element = {
            .handle = destroy_rendering_device
        };

        FCG_Memory_InitializeStack(&element.arguments);
        FCG_Memory_PushStack(&element.arguments, instance->rendering_devices, sizeof(FCG_RenderingDevice));

        FCG_Memory_PushStack(&machine->destructor_stack, &element, sizeof(FCG_DestructorElement));
    }

    switch (surface->type)
    {
    case FCG_SURFACE_WINDOW: 
    {
        VkResult result = create_swap_chain(instance, surface);
        if (result != VK_SUCCESS) return FCG_SWAPCHAIN_ERROR;

        /* Handle swapchain destruction */
        {
            FCG_DestructorElement element = {
                .handle = destroy_swapchain
            };

            FCG_Memory_InitializeStack(&element.arguments);
            FCG_Memory_PushStack(&element.arguments, &surface->surface_image.handle, sizeof(FCG_Handle));
            FCG_Memory_PushStack(&element.arguments, &instance->rendering_devices->handle, sizeof(FCG_Handle));

            FCG_Memory_PushStack(&machine->destructor_stack, &element, sizeof(FCG_DestructorElement));
        }
    }
    default: break;
    }

    return FCG_SUCCESS;
}

FCG_Result 
FCG_DestroyGraphicsInstance(
    FCG_GDI* FCG_CR instance)
{
    FCG_assert(instance->active);

    /* We are good to free the list, copies of the needed handles for  */
    /* destruction are held inside the argument stacks attached to the */
    /* relevant destructor function pointers.                          */
    if (instance->rendering_devices) free(instance->rendering_devices);

    memset(instance, 0, sizeof(FCG_GDI));
    return FCG_SUCCESS;
}