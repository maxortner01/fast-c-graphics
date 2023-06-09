#ifndef TYPES_H
#define TYPES_H

#include <stddef.h>
#include <stdint.h>

#ifndef FCG_RENDER_API_TYPE
#   error "Render API type not specified, rebuild and choose OpenGL or Vulkan."
#else
#   if FCG_RENDER_API_TYPE == FCG_OPENGL
#   elif FCG_RENDER_API_TYPE == FCG_VULKAN
#   else
#       error "Render API not supported."
#   endif
#endif

/* Argument attributes */
#define FCG_CR const restrict

#define FCG_Bool  uint8_t
#define FCG_True  1
#define FCG_False 0
#define FCG_Iterator char*
#define FCG_Handle void*
#define FCG_DataHandle void*

#define ADDR_DIFF ptrdiff_t
#define SIZE size_t
#define U32 uint32_t
#define I32 int32_t

#define CSIZE const SIZE
#define CU32 const U32
#define CI32 const I32

typedef enum
{
    FCG_SUCCESS,
    FCG_INIT_FAILED,
    FCG_WINDOW_CREATE_FAILED,
    FCG_CONTEXT_CREATE_FAILED,
    FCG_INSTANCE_CREATE_FAILED,
    FCG_RENDERING_DEVICE_FAILED,
    FCG_DEBUG_ERROR,
    FCG_SWAPCHAIN_ERROR,
    FCG_IMAGE_VIEW_ERROR,
    FCG_RENDER_PASS_FAILED,
    FCG_FRAMEBUFFER_FAILED,
    FCG_SHADER_COMPILE_FAILED,
    FCG_SHADER_COMPILATION_NOT_SUPPORTED,
    FCG_SHADER_FILE_TYPE_NOT_SUPPORTED,
    FCG_SHADER_TYPE_NOT_SUPPORTED,
} FCG_Result;

#endif