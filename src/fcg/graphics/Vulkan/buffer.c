#include <fcg/graphics/buffer.h>
#include "../../assert.c"

#include <vk_mem_alloc.h>

typedef struct Allocation_s
{
    FCG_Handle buffer;
    FCG_Handle allocation;
} Allocation;

Allocation
allocate_GPU_memory(
    FCG_Handle owner,
    void** ptr, 
    U32    size)
{
    VkBufferCreateInfo create_info = {
        .sType                 = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext                 = NULL,
        .flags                 = 0,
        .pQueueFamilyIndices   = NULL,
        .queueFamilyIndexCount = 0,
        .sharingMode           = 0,
        .size                  = size,
        .usage                 = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT
    };

    VmaAllocationCreateInfo alloc_create_info = {
        .flags          = VMA_ALLOCATION_CREATE_MAPPED_BIT,
        .memoryTypeBits = 0,
        .pool           = 0,
        .preferredFlags = 0,
        .priority       = 0.f,
        .pUserData      = NULL,
        .requiredFlags  = 0,
        .usage          = VMA_MEMORY_USAGE_CPU_TO_GPU
    };

    VkBuffer      buffer;
    VmaAllocation allocation;
    FCG_assert(vmaCreateBuffer(owner, &create_info, &alloc_create_info, &buffer, &allocation, NULL) == VK_SUCCESS);

    // map data
    vmaMapMemory(owner, allocation, ptr);

    Allocation ret = { .buffer = buffer, .allocation = allocation };
    return ret;
}

void 
deallocate_GPU_memory(
    FCG_Handle owner,
    FCG_Handle allocation,
    FCG_Handle buffer)
{
    // Unmap data
    vmaUnmapMemory(owner, allocation);

    vmaDestroyBuffer(owner, buffer, allocation);
}

FCG_Result
FCG_Data_InitLayout(
    FCG_Data_BufferLayout* FCG_CR layout)
{
    memset(layout, 0, sizeof(FCG_Data_BufferLayout));
    FCG_Memory_InitializeQueue(&layout->buffers);
    return FCG_SUCCESS;
}

FCG_Result
FCG_Data_DestroyLayout(
    FCG_Data_BufferLayout* FCG_CR layout)
{
    for (U32 i = 0; i < layout->buffers.object_count; i++)
        FCG_assert(FCG_Data_DestroyBuffer(FCG_Memory_QueueGet(&layout->buffers, i)) == FCG_SUCCESS);

    return FCG_SUCCESS;
}

FCG_Data_Buffer*
FCG_Data_CreateBuffer(
    const FCG_Machine* FCG_CR machine,
    FCG_Data_BufferLayout* FCG_CR layout)
{
    FCG_Data_Buffer buffer;
    memset(&buffer, 0, sizeof(FCG_Data_Buffer));

    buffer.owner = machine->allocator;

    return FCG_Memory_PushQueue(&layout->buffers, &buffer, sizeof(FCG_Data_Buffer));
}

void
FCG_Data_LoadData(
    FCG_Data_Buffer* FCG_CR buffer,
    void* data,
    CU32  block_size,
    U32   attribute_count,
    FCG_Data_Attribute* attributes)
{
    FCG_Handle allocator = buffer->owner;
    
    /* If the data size has changed, just destroy, this will free the pointers and leave the memory set to 0 */
    if (buffer->allocated != block_size)
    {
        FCG_assert(FCG_Data_DestroyBuffer(buffer) == FCG_SUCCESS);
        buffer->owner = allocator;

        // allocate the data
        buffer->allocated = block_size;
        Allocation alloc = allocate_GPU_memory(buffer->owner, &buffer->data, buffer->allocated);
        buffer->buffer = alloc.buffer;
        buffer->allocation = alloc.allocation;
    }

    /* If the attributes have changed, reallocate to accomodate */
    if (buffer->attribute_count != attribute_count)
    {
        buffer->attributes = realloc(buffer->attributes, sizeof(FCG_Data_Attribute) * attribute_count);
        buffer->attribute_count = attribute_count;
    }

    /* Copy over the attributes if they're new */
    if (buffer->attributes != attributes)
        memcpy(buffer->attributes, attributes, sizeof(FCG_Data_Attribute) * attribute_count);

    /* Finally, copy over the data */
    FCG_assert(buffer->data);
    memcpy(buffer->data, data, block_size);
}

FCG_Result
FCG_Data_DestroyBuffer(
    FCG_Data_Buffer* FCG_CR buffer)
{
    if (buffer->data)
        deallocate_GPU_memory(buffer->owner, buffer->allocation, buffer->buffer);
    
    if (buffer->attributes) free(buffer->attributes);

    memset(buffer, 0, sizeof(FCG_Data_Buffer));

    return FCG_SUCCESS;
}