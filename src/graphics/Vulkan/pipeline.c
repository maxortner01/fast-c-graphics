#include <fcg/graphics/pipeline.h>

#if defined(USE_SHADERC) && FCG_RENDER_API_TYPE == FCG_VULKAN
#   include <shaderc/shaderc.h>
#   define USE_SHADERC_AND_VULKAN
#endif

// Where the magic happens
FCG_Result
FCG_Module_ConstructPipeline(
    FCG_Module_Transformation* FCG_CR pipeline,
    FCG_Module_PipelineInfo*   FCG_CR pipeline_info,
    FCG_Surface*               FCG_CR surface)
{

}