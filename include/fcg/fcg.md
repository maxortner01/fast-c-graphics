Here's how we want it to play out. There's data, then there's a surface. To get the data to the surface we need transformations. Thus, the user constructs transformation pipelines that start with data and end with the surface. The pipeline consists of stages that are configured/created by the user.

```c++
void func(FCG_Data_Buffer* FCG_CR buffer);

FCG_Transformation_Layout layout;

FCG_Surface surface;
FCG_Transformation_LayoutTerminal(&layout, &surface);

FCG_Data_Buffer     buffer;
FCG_Data_BufferInfo buffer_info;
FCG_Data_CreateBuffer(&buffer, &buffer_info);
FCG_Transformation_LayoutRoot(&layout, &buffer);

FCG_Module_Programmable shaders[];
shaders[0].type = FCG_SHADER_VERTEX;
shaders[0].filename = "vertex.glsl";

shaders[1].type = FCG_SHADER_FRAGMENT;
shaders[1].filename = "fragment.glsl";

FCG_Module_Transformation pipeline;
FCG_Module_PipelineInfo pipeline_info = {
    .shaderCount = 2,
    .shaders     = &shaders[0]
};

FCG_Module_ConstructPipeline(&pipeline, &pipeline_info);
FCG_Transformation_Push(&layout, &pipeline.transformation);

FCG_Module_Function function = {
    .handle = func
};

FCG_Transformation_Push(&layout, &function_info);
```