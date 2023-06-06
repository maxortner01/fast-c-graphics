#include <fcg/graphics/pipeline.h>

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
FCG_Transformation_Push(
    FCG_Transformation_Layout* FCG_CR layout,
    const FCG_Module_Transformation* FCG_CR module)
{
    FCG_Memory_PushQueue(&layout->stages, module, sizeof(FCG_Module_Transformation));
}