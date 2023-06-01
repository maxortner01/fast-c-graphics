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
    FCG_SUCCESS                 = 0x00,
    FCG_INIT_FAILED             = 0x01,
    FCG_WINDOW_CREATE_FAILED    = 0x02,
    FCG_CONTEXT_CREATE_FAILED   = 0x04,
    FCG_INSTANCE_CREATE_FAILED  = 0x08,
    FCG_RENDERING_DEVICE_FAILED = 0x10,
    FCG_DEBUG_ERROR             = 0x20,
    FCG_SWAPCHAIN_ERROR         = 0x40
} FCG_Result;

#endif