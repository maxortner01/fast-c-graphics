#include <fcg/graphics/gdi.h>
#include "../../assert.c"

#include <vulkan/vulkan.h>

#define INITIALIZE(type, name) type name; memset(&name, 0, sizeof(type))

void 
destroy_rendering_device(
    FCG_Memory_Stack* FCG_CR stack)
{
    printf("destroying logic device\n");

    FCG_RenderingDevice device;
    FCG_Memory_PopStack(stack, &device);

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