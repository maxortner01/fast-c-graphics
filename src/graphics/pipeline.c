#include <fcg/graphics/pipeline.h>
#include "../assert.c"

#include <string.h>

FCG_Result
FCG_Transformation_InitLayout(
    FCG_Transformation_Layout* FCG_CR layout)
{
    memset(layout, 0, sizeof(FCG_Transformation_Layout));
    FCG_Memory_InitializeQueue(&layout->stages);
    return FCG_SUCCESS;
}

FCG_Result
FCG_Transformation_DestroyLayout(
    FCG_Transformation_Layout* FCG_CR layout)
{
    FCG_Memory_DestroyQueue(&layout->stages);
}

FCG_Result
FCG_Transformation_ConstructLayout(
    FCG_Transformation_Layout* FCG_CR layout)
{
    FCG_Data_BufferLayout* previous_layout = layout->root;
    FCG_Surface* next_surface = layout->terminal;

    for (U32 i = 0; i < layout->stages.object_count; i++)
    {
        FCG_Module_Transformation* stage = FCG_Memory_QueueGet(&layout->stages, i);
        switch(stage->transform_type)
        {
        case FCG_TRANSFORM_TYPE_PIPELINE: FCG_assert(FCG_Module_ConstructPipeline(stage, stage->create_info, previous_layout, next_surface) == FCG_SUCCESS); break;
        default: FCG_assert(FCG_False);
        }
    }
}

FCG_Result 
FCG_Transformation_Push(
    FCG_Transformation_Layout* FCG_CR layout,
    const FCG_Module_Transformation* FCG_CR module)
{
    FCG_Memory_PushQueue(&layout->stages, module, sizeof(FCG_Module_Transformation));
}