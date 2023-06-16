#include <fcg/graphics/pipeline.h>
#include <fcg/graphics/gdi.h>
#include <fcg/graphics/surface.h>
#include <fcg/graphics/buffer.h>
#include <stdio.h>
#include "../../assert.c"

#include <vulkan/vulkan.h>

#if defined(USE_SHADERC) && FCG_RENDER_API_TYPE == FCG_VULKAN
#   include <shaderc/shaderc.h>
#   define USE_SHADERC_AND_VULKAN
#endif

FCG_Handle
create_shader_from_sprv(
    FCG_Module_Programmable* FCG_CR shader,
    const U32*  data, 
    CU32        count)
{
    VkShaderModuleCreateInfo create_info = {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .codeSize = count * sizeof(U32),
        .pCode = data
    };

    VkShaderModule module;
    VkResult result = vkCreateShaderModule(shader->device->handle, &create_info, NULL, &module);
    if (result != VK_SUCCESS) return NULL;

    return (FCG_Handle)module;
}   

/* Not sure how to do this in C... seems like all the APIs are in C++ */
FCG_Handle
compile_glsl(
    FCG_Module_Programmable* FCG_CR shader,
    const char*                     contents,
    CU32                            size)
{
    return NULL;
/*
    shaderc_compiler_t compiler;
    shaderc_compile_options_t compile_options;

    shaderc_shader_kind kind;
    switch(shader->type)
    {
    case FCG_SHADER_VERTEX:   kind = shaderc_vertex_shader;   break;
    case FCG_SHADER_FRAGMENT: kind = shaderc_fragment_shader; break;
    default: return NULL;
    }

    shaderc_compile_options_set_optimization_level(compile_options, shaderc_optimization_level_size);
    //shaderc_compile_into_preprocessed_text(compiler, contents, size, kind, shader->filename, "main", compile_options);
    shaderc_compilation_result_t result = shaderc_compile_into_spv(
        compiler, 
        contents, 
        size, 
        kind, 
        shader->filename, 
        "main", 
        compile_options
    );

    if (shaderc_result_get_num_errors(result))
    {
        printf("Error compiling '%s'.\n%s\n", shader->filename, shaderc_result_get_error_message(result));

        shaderc_compiler_release(compiler);
        shaderc_compile_options_release(compile_options);
        shaderc_result_release(result);
        return NULL;
    }
    
    const char* bytes     = shaderc_result_get_bytes(result);
    size_t      byte_size = shaderc_result_get_length(result);

    FCG_Handle handle = create_shader_from_sprv(bytes, byte_size);

    shaderc_compiler_release(compiler);
    shaderc_compile_options_release(compile_options);
    shaderc_result_release(result);

    return handle;*/
}

FCG_Result
compile_shader(
    FCG_Module_Programmable* FCG_CR shader)
{
    if (shader->file_type == FCG_SHADER_FILE_TYPE_GLSL)
    {
#   if !defined(USE_SHADERC) && FCG_RENDER_API == FCG_VULKAN
        return FCG_SHADER_COMPILATION_NOT_SUPPORTED;
#   endif
    }

    FILE* file = fopen(shader->filename, "rb");
    FCG_assert(file);

    /* Get the filesize (probably a better way to do this) */
    long file_size = 0;
    while (fgetc(file) != EOF) file_size++;

    /* Copy the data over */
    CU32 count = (U32)(file_size / sizeof(U32));
    U32* contents = (U32*)calloc(count, sizeof(U32));
    rewind(file);
    fread(contents, sizeof(U32), count, file);
    fclose(file);

    /* Now we compile */
    FCG_Handle handle = NULL;
    switch (shader->file_type)
    {
    //case FCG_SHADER_FILE_TYPE_GLSL: handle = compile_glsl(shader, contents, count);    break;
    case FCG_SHADER_FILE_TYPE_SPRV: handle = create_shader_from_sprv(shader, contents, count); break;
    default: free(contents); return FCG_SHADER_FILE_TYPE_NOT_SUPPORTED;
    }

    free(contents);

    shader->handle = handle;

    if (!handle) return FCG_SHADER_COMPILE_FAILED;

    return FCG_SUCCESS;
}

#define INIT_STRUCT(type, name) type name; memset(&name, 0, sizeof(type))

// Where the magic happens
FCG_Result
FCG_Module_ConstructPipeline(
    FCG_Module_Transformation* FCG_CR pipeline,
    FCG_Module_PipelineInfo*   FCG_CR pipeline_info,
    FCG_Data_BufferLayout*     FCG_CR buffers,
    FCG_Surface*               FCG_CR surface)
{
    VkPipelineShaderStageCreateInfo* stage_create_infos = calloc(
        pipeline_info->shader_count,
        sizeof(VkPipelineShaderStageCreateInfo));

    /* Load the pre-compiled shaders in and set up the stage create info */
    for (uint32_t i = 0; i < pipeline_info->shader_count; i++)
    {
        FCG_Result result = compile_shader(pipeline_info->shaders + i);
        if (result != FCG_SUCCESS) { free(stage_create_infos); return result; }

        stage_create_infos[i].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stage_create_infos[i].module = pipeline_info->shaders[i].handle;

        switch (pipeline_info->shaders[i].type)
        {
        case FCG_SHADER_VERTEX:   stage_create_infos[i].stage = VK_SHADER_STAGE_VERTEX_BIT;   break;
        case FCG_SHADER_FRAGMENT: stage_create_infos[i].stage = VK_SHADER_STAGE_FRAGMENT_BIT; break;
        default: free(stage_create_infos); return FCG_SHADER_TYPE_NOT_SUPPORTED;
        }

        stage_create_infos[i].pName = "main";
    }

    /* Bindings */
    U32 total_attributes = 0;
    VkVertexInputBindingDescription* binding = calloc(buffers->buffers.object_count, sizeof(VkVertexInputBindingDescription));
    for (U32 i = 0; i < buffers->buffers.object_count; i++)
    {
        U32 stride = 0;
        FCG_Data_Buffer* buffer = FCG_Memory_QueueGet(&buffers->buffers, i);
        total_attributes += buffer->attribute_count;
        for (U32 j = 0; j < buffer->attribute_count; j++)
        {
            FCG_Data_Attribute* attribute = buffer->attributes + j;
            stride += attribute->element_size * attribute->element_count;
        }

        binding[i].binding = i;
        binding[i].stride  = stride;
        binding[i].inputRate = 0; // ?
    }

    /* Attributes */
    U32 index = 0;
    VkVertexInputAttributeDescription* attributes = calloc(total_attributes, sizeof(VkVertexInputAttributeDescription));
    for (U32 i = 0; i < buffers->buffers.object_count; i++)
    {
        FCG_Data_Buffer* buffer = FCG_Memory_QueueGet(&buffers->buffers, i);

        U32 offset = 0;
        for (U32 j = 0; j < buffer->attribute_count; j++)
        {
            FCG_Data_Attribute* attribute = buffer->attributes + j;
            VkFormat format;

            if (attribute->element_size == 4)
            {
                switch (attribute->element_count)
                {
                case 1: format = VK_FORMAT_R32_SFLOAT; break;
                case 2: format = VK_FORMAT_R32G32_SFLOAT; break;
                case 3: format = VK_FORMAT_R32G32B32_SFLOAT; break;
                case 4: format = VK_FORMAT_R32G32B32A32_SFLOAT; break;
                default: FCG_assert(FCG_False);
                }
            }
            else FCG_assert(FCG_False);

            attributes[index  ].location = index;
            attributes[index  ].format   = format;
            attributes[index  ].binding  = i;
            attributes[index++].offset   = offset;

            offset += attribute->element_count * attribute->element_size;
        }
    }

    /* Take in the attributes of the buffer set */
    INIT_STRUCT(VkPipelineVertexInputStateCreateInfo, input_create_info);
    input_create_info.vertexBindingDescriptionCount = buffers->buffers.object_count;
    input_create_info.pVertexBindingDescriptions = binding;
    input_create_info.vertexAttributeDescriptionCount = total_attributes;
    input_create_info.pVertexAttributeDescriptions = attributes;
    input_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    INIT_STRUCT(VkPipelineInputAssemblyStateCreateInfo, input_assembly_create_info);
    input_assembly_create_info.sType    = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    input_assembly_create_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    VkDynamicState dynamic_states[] = { VK_DYNAMIC_STATE_SCISSOR, VK_DYNAMIC_STATE_VIEWPORT };

    VkViewport viewport = {
        .x = 0.f,
        .y = 0.f,
        .width = surface->size.width,
        .height = surface->size.height,
        .maxDepth = 1.f,
        .minDepth = 0.f
    };

    VkRect2D scissor = {
        .extent = {
            .width = surface->size.width,
            .height = surface->size.height
        },
        .offset = { .x = 0.f, .y = 0.f }
    };

    /* Rastor info (debug info/culling etc.) */
    INIT_STRUCT(VkPipelineRasterizationStateCreateInfo, raster_create_info);
    raster_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    raster_create_info.polygonMode = VK_POLYGON_MODE_FILL;
    raster_create_info.lineWidth = 1.f;
    raster_create_info.cullMode = VK_CULL_MODE_NONE;
	raster_create_info.frontFace = VK_FRONT_FACE_CLOCKWISE;

    /* Multisampling (not used currently) */
    INIT_STRUCT(VkPipelineMultisampleStateCreateInfo, multisample_create_info);
    multisample_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisample_create_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisample_create_info.minSampleShading = 1.0f;

    /* Blend info */
    INIT_STRUCT(VkPipelineColorBlendAttachmentState, color_blend_state);
    color_blend_state.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
			VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

    INIT_STRUCT(VkPipelineViewportStateCreateInfo, viewport_create_info);
    viewport_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewport_create_info.viewportCount = 1;
    viewport_create_info.pViewports = &viewport;
    viewport_create_info.scissorCount = 1;
    viewport_create_info.pScissors = &scissor;

    INIT_STRUCT(VkPipelineColorBlendStateCreateInfo, blending_create_info);
    blending_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    blending_create_info.logicOp = VK_LOGIC_OP_COPY;
    blending_create_info.attachmentCount = 1;
    blending_create_info.pAttachments = &color_blend_state;

    VkPipelineLayout layout;
    INIT_STRUCT(VkPipelineLayoutCreateInfo, pipeline_create_info);
    pipeline_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_create_info.setLayoutCount = 0; // Optional
    pipeline_create_info.pSetLayouts = NULL; // Optional
    pipeline_create_info.pushConstantRangeCount = 0; // Optional
    pipeline_create_info.pPushConstantRanges = NULL; // Optional

    FCG_assert(vkCreatePipelineLayout(pipeline_info->gdi->rendering_devices->handle, &pipeline_create_info, NULL, &layout) == VK_SUCCESS)

    VkGraphicsPipelineCreateInfo create_info = {
        .sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .pNext               = NULL,
        .stageCount          = pipeline_info->shader_count,
        .pStages             = stage_create_infos,
        .pVertexInputState   = &input_create_info,
        .pInputAssemblyState = &input_assembly_create_info,
        .pViewportState      = &viewport_create_info,
        .pRasterizationState = &raster_create_info,
        .pMultisampleState   = &multisample_create_info,
        .pColorBlendState    = &blending_create_info,
        .layout              = layout,
        .renderPass          = surface->surface_image.pass_handle,
        .subpass             = 0,
        .basePipelineHandle  = VK_NULL_HANDLE
    };

    VkPipeline vk_pipeline;
    FCG_assert(vkCreateGraphicsPipelines(pipeline_info->gdi->rendering_devices->handle, NULL, 1, &create_info, NULL, &vk_pipeline) == VK_SUCCESS);
	
    pipeline->handle = vk_pipeline;

    free(attributes);
    free(binding);
    free(stage_create_infos);

    return FCG_SUCCESS;
}

#undef INIT_STRUCT