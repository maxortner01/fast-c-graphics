#include <fcg/graphics/surface.h>
#include <fcg/graphics/gdi.h>
#include <fcg/graphics/device.h>
#include "../../assert.c"

#include "instance.h"

#include <vulkan/vulkan.h>

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
    const VkSurfaceKHR               vk_surface      = surface->context;
    const FCG_RenderingDevice* const render_device   = gdi->rendering_devices;
    const FCG_GraphicsDevice*  const graphics_device = render_device->suitable_device;
    const VkPhysicalDevice           physical_device = render_device->suitable_device->handle;

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
    surface->surface_image.format = surface_format.format;
    memcpy(&surface->surface_image.size, &extent, sizeof(U32) * 2);

    free(formats); free(present_modes);
    return result;
}

void 
destroy_swapchain(
    FCG_Memory_Stack* FCG_CR stack)
{
    printf("destroying swapchain\n");

    FCG_Handle device; FCG_Handle swapchain;
    FCG_Memory_PopStack(stack, &device);
    FCG_Memory_PopStack(stack, &swapchain);

    vkDestroySwapchainKHR(device, swapchain, NULL);
}

void 
destroy_image_views(
    FCG_Memory_Stack* FCG_CR stack)
{
    printf("destroying image views\n");

    FCG_Handle device;
    FCG_Memory_PopStack(stack, &device);

    while (stack->object_count)
    {
        FCG_Handle view;
        FCG_Memory_PopStack(stack, &view);
        vkDestroyImageView(device, view, NULL);
    }
}

FCG_Result 
generate_image_views(
    const VkImage* FCG_CR images, 
    FCG_Surface* FCG_CR   surface,
    const FCG_GDI* FCG_CR instance)
{
    FCG_SurfaceImage* image = &surface->surface_image;
    image->images = calloc(image->image_count, sizeof(FCG_Image)); // freed in FCG_Surface_Destroy(...)

    FCG_DestructorElement element = {
        .handle = destroy_image_views
    };

    FCG_Memory_InitializeStack(&element.arguments);
    /* Copy the handles over to the corresponding image location */
    for (U32 i = 0; i < image->image_count; i++)
    {
        image->images[i].image = images[i]; // fun line of code
        
        // Generate image view
        VkImageViewCreateInfo create_info = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .pNext = NULL,
            .components = {
                .r = VK_COMPONENT_SWIZZLE_IDENTITY,
                .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                .a = VK_COMPONENT_SWIZZLE_IDENTITY
            },
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format   = image->format,
            .subresourceRange = {
                .aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseArrayLayer = 0,
                .baseMipLevel   = 0, 
                .levelCount     = 1,
                .layerCount     = 1
            },
            .flags = 0,
            .image = images[i],
        };

        VkImageView view;
        VkResult result = vkCreateImageView(instance->rendering_devices->handle, &create_info, NULL, &view);
        if (result != VK_SUCCESS) return FCG_IMAGE_VIEW_ERROR;

        image->images[i].view = view;
        FCG_Memory_PushStack(&element.arguments, &image->images[i].view, sizeof(FCG_Handle));
    }

    /* Push the logic device to the top of the stack*/
    FCG_Memory_PushStack(&element.arguments, &instance->rendering_devices->handle, sizeof(FCG_Handle));
    FCG_Memory_PushStack(&surface->destructor_stack, &element, sizeof(FCG_DestructorElement));
    printf("Pushing image views\n");
}

void 
destroy_render_pass(
    FCG_Memory_Stack* FCG_CR stack)
{
    printf("destroying render pass\n");

    FCG_Handle device; FCG_Handle render_pass;
    FCG_Memory_PopStack(stack, &device);
    FCG_Memory_PopStack(stack, &render_pass);

    vkDestroyRenderPass(device, render_pass, NULL);
}

FCG_Result
generate_render_pass(    
    FCG_Surface* FCG_CR surface,
    const FCG_GDI* FCG_CR instance)
{
    VkAttachmentDescription attachment_description = {
        .format         = surface->surface_image.format,
        .samples        = VK_SAMPLE_COUNT_1_BIT,
        .loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp        = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
    };

    VkAttachmentReference attachment_ref = {
        .attachment = 0,
        .layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    };

    VkSubpassDescription subpass_desc;
    memset(&subpass_desc, 0, sizeof(VkSubpassDescription));

    subpass_desc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass_desc.colorAttachmentCount = 1;
    subpass_desc.pColorAttachments = &attachment_ref;

    VkSubpassDependency subpass_dependency = {
        .srcSubpass = VK_SUBPASS_EXTERNAL,
        .dstSubpass = 0,
        .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .srcAccessMask = 0,
        .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
    };

    VkRenderPassCreateInfo create_info = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .attachmentCount = 1,
        .pAttachments = &attachment_description,
        .subpassCount = 1,
        .pSubpasses = &subpass_desc,
        .dependencyCount = 1,
        .pDependencies = &subpass_dependency
    };

    VkRenderPass render_pass;
    VkResult result = vkCreateRenderPass(
        instance->rendering_devices->handle,
        &create_info,
        NULL,
        &render_pass);
    if (result != VK_SUCCESS) return FCG_RENDER_PASS_FAILED;

    surface->surface_image.pass_handle = render_pass;

    FCG_DestructorElement element = {
        .handle = destroy_render_pass
    };

    FCG_Memory_InitializeStack(&element.arguments);
    FCG_Memory_PushStack(&element.arguments, &surface->surface_image.pass_handle,  sizeof(FCG_Handle));
    FCG_Memory_PushStack(&element.arguments, &instance->rendering_devices->handle, sizeof(FCG_Handle));
    FCG_Memory_PushStack(&surface->destructor_stack, &element, sizeof(FCG_DestructorElement));
    printf("Pushing render pass\n");

    return FCG_SUCCESS;
}

void 
destroy_framebuffers(
    FCG_Memory_Stack* FCG_CR stack)
{
    printf("destroying framebuffers\n");

    FCG_Handle device;
    FCG_Memory_PopStack(stack, &device);

    while (stack->object_count)
    {
        FCG_Handle framebuffer = NULL;
        FCG_Memory_PopStack(stack, &framebuffer);
        vkDestroyFramebuffer(device, framebuffer, NULL);
    }
}

FCG_Result
generate_frame_buffers(    
    FCG_Surface* FCG_CR surface,
    const FCG_GDI* FCG_CR instance)
{
    FCG_SurfaceImage* image = &surface->surface_image;

    FCG_DestructorElement element = {
        .handle = destroy_framebuffers
    };

    FCG_Memory_InitializeStack(&element.arguments);
    for (U32 i = 0; i < image->image_count; i++)
    {
        VkFramebufferCreateInfo create_info = {
            .sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .renderPass      = image->pass_handle,
            .attachmentCount = 1,
            .pAttachments    = &image->images[i].view,
            .width           = surface->size.width,
            .height          = surface->size.height,
            .layers          = 1,
            .flags           = 0,
            .pNext           = NULL
        };

        VkFramebuffer framebuffer;
        VkResult result = vkCreateFramebuffer(instance->rendering_devices->handle, &create_info, NULL, &framebuffer);
        if (result != VK_SUCCESS) return FCG_FRAMEBUFFER_FAILED;

        image->images[i].frame_buffer = framebuffer;
        FCG_Memory_PushStack(&element.arguments, &image->images[i].frame_buffer, sizeof(FCG_Handle));
    }

    FCG_Memory_PushStack(&element.arguments, &instance->rendering_devices->handle, sizeof(FCG_Handle));
    FCG_Memory_PushStack(&surface->destructor_stack, &element, sizeof(FCG_DestructorElement));
    printf("pushing frame buffers\n");

    return FCG_SUCCESS;
}

FCG_Result
FCG_Frame_Create(
    FCG_Frame* FCG_CR frame,
    const FCG_Surface* FCG_CR surface,
    FCG_GDI* FCG_CR gdi)
{
    VkFenceCreateInfo fence_create_info = {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .pNext = NULL,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT
    };

    VkFence render_fence;
    FCG_assert(vkCreateFence(gdi->rendering_devices->handle, &fence_create_info, NULL, &render_fence) == VK_SUCCESS);

    VkSemaphoreCreateInfo semaphore_create_info = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        .pNext = NULL,
        .flags = 0
    };

    VkSemaphore render_semaphore;
    VkSemaphore present_semaphore;
    FCG_assert(vkCreateSemaphore(gdi->rendering_devices->handle, &semaphore_create_info, NULL, &render_semaphore) == VK_SUCCESS);
    FCG_assert(vkCreateSemaphore(gdi->rendering_devices->handle, &semaphore_create_info, NULL, &present_semaphore) == VK_SUCCESS);

    memset(frame, 0, sizeof(FCG_Frame));
    frame->present_semaphore = present_semaphore;
    frame->render_semaphore  = render_semaphore;
    frame->render_fence      = render_fence;
    frame->surface           = surface;
}

FCG_Result 
FCG_Surface_Initialize(
    FCG_Surface* FCG_CR surface, 
    FCG_Machine* FCG_CR machine,
    const FCG_GDI* FCG_CR instance)
{
    VkImage* images = NULL;

    /* Create the vk surface */
    VkSurfaceKHR vulkan_surface;
    FCG_assert(SDL_Vulkan_CreateSurface(surface->handle, machine->handle, &vulkan_surface));
    surface->context = vulkan_surface; 

    /* Handle vulkan surface destruction */
    {
        FCG_DestructorElement element = {
            .handle    = destroy_surface
        };

        FCG_Memory_InitializeStack(&element.arguments);
        FCG_Memory_PushStack(&element.arguments, &surface->context, sizeof(FCG_Handle));
        FCG_Memory_PushStack(&element.arguments, &machine->handle,  sizeof(FCG_Handle));

        FCG_Memory_PushStack(&machine->destructor_stack, &element, sizeof(FCG_DestructorElement));
    }

    /* Handle FCG_surface destruction */
    {
        FCG_DestructorElement element = {
            .handle = FCG_Surface_Destroy
        };

        FCG_Memory_InitializeStack(&element.arguments);
        FCG_Memory_PushStack(&element.arguments, &surface, sizeof(FCG_Handle));

        FCG_Memory_PushStack(&machine->destructor_stack, &element, sizeof(FCG_DestructorElement));
        printf("pushing surface\n");
    }

    /* Move this into its own surface_register function or something */
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

            FCG_Memory_PushStack(&surface->destructor_stack, &element, sizeof(FCG_DestructorElement));
            printf("pushing swapchain\n");
        }

        /* Generate image views */
        {
            FCG_SurfaceImage* image = &surface->surface_image;
            image->index = 0;

            vkGetSwapchainImagesKHR(
                instance->rendering_devices->handle, 
                image->handle, 
                &image->image_count, 
                NULL);

            images = calloc(surface->surface_image.image_count, sizeof(VkImage)); // freed at end of scope
            vkGetSwapchainImagesKHR(
                instance->rendering_devices->handle, 
                image->handle, 
                &image->image_count, 
                images);
        }
    }
    default: break;
    }

    FCG_assert(images);

    FCG_Result result;
    /* Generate render pass */
    FCG_assert(generate_render_pass(surface, instance) == FCG_SUCCESS);
    //if (result) { free(images); return result; }

    /* Generate image views */
    FCG_assert(generate_image_views(images, surface, instance) == FCG_SUCCESS);
    //if (result) { free(images); return result; }

    /* Generate frame buffers */
    FCG_assert(generate_frame_buffers(surface, instance) == FCG_SUCCESS);
    //if (result) { free(images); return result; }

    free(images);
    return FCG_SUCCESS;   
}