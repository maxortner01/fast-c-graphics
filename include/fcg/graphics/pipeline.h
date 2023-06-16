#ifndef PIPELINE_H_
#define PIPELINE_H_

#include "../memory/queue.h"
#include "../types.h"

typedef struct FCG_RenderingDevice_s   FCG_RenderingDevice;
typedef struct FCG_Surface_s           FCG_Surface;
typedef struct FCG_GDI_s               FCG_GDI;
typedef struct FCG_Data_BufferLayout_s FCG_Data_BufferLayout;

/**
 * @brief Represents the various types of programmable shaders.
 */
typedef enum FCG_ShaderType_e
{
    FCG_SHADER_NONE,
    FCG_SHADER_VERTEX,
    FCG_SHADER_FRAGMENT
} FCG_ShaderType;

typedef enum FCG_ShaderFileType_s
{
    FCG_SHADER_FILE_TYPE_NONE,
    FCG_SHADER_FILE_TYPE_GLSL,
    FCG_SHADER_FILE_TYPE_SPRV
} FCG_ShaderFileType;

/**
 * @brief Types of transformations that can go in a layout.
 */
typedef enum FCG_TransformType_s
{
    FCG_TRANSFORM_TYPE_NONE,
    FCG_TRANSFORM_TYPE_PIPELINE
} FCG_TransformType;

/**
 * @brief The layout of a transformation pipeline.
 */
typedef struct FCG_Transformation_Layout_s
{
    FCG_Data_BufferLayout* root;
    FCG_Memory_Queue       stages; // < really should change this to block allocation
    FCG_Surface*           terminal;
} FCG_Transformation_Layout;

/**
 * @brief Represents a transformation stage in a layout.
 */
typedef struct FCG_Module_Transformation_s
{
    FCG_Handle        handle;
    FCG_TransformType transform_type;
    void*             create_info;
} FCG_Module_Transformation;

/**
 * @brief A programmable shader module.
 */
typedef struct FCG_Module_Programmable_s
{
    FCG_Handle           handle;
    const char*          filename;
    FCG_ShaderType       type;
    FCG_ShaderFileType   file_type;
    FCG_RenderingDevice* device;
} FCG_Module_Programmable;

/**
 * @brief The construction information for a transformation of type FCG_TRANSFORM_TYPE_PIPELINE.
 */
typedef struct FCG_Module_PipelineInfo_s
{
    uint32_t                 shader_count;
    FCG_Module_Programmable* shaders;
    FCG_GDI*                 gdi;
} FCG_Module_PipelineInfo;

/**
 * @brief Initializes a transformation layout.
 * 
 * @param layout The transformation layout
 * @return FCG_Result 
 */
FCG_SHARED
FCG_Result
FCG_Transformation_InitLayout(
    FCG_Transformation_Layout* FCG_CR layout);

FCG_SHARED
FCG_Result
FCG_Transformation_DestroyLayout(
    FCG_Transformation_Layout* FCG_CR layout);

/* After all the stages have been added, you can construct the layout */
FCG_SHARED
FCG_Result
FCG_Transformation_ConstructLayout(
    FCG_Transformation_Layout* FCG_CR layout);

/**
 * @brief Pushes a stage into the given layout.
 * 
 * @param layout Transformation layout
 * @param module Module to push into layout
 * @return FCG_Result 
 */
FCG_SHARED
FCG_Result 
FCG_Transformation_Push(
    FCG_Transformation_Layout* FCG_CR layout,
    const FCG_Module_Transformation* FCG_CR module);

/**
 * @brief Constructs a graphics pipeline from the given shaders.
 * 
 * @param pipeline The pipeline transformation module to use
 * @param pipeline_info The creation information
 * @return FCG_Result
 */
FCG_SHARED
FCG_Result
FCG_Module_ConstructPipeline(
    FCG_Module_Transformation* FCG_CR pipeline,
    FCG_Module_PipelineInfo*   FCG_CR pipeline_info,
    FCG_Data_BufferLayout*     FCG_CR buffers,
    FCG_Surface*               FCG_CR surface);

#endif