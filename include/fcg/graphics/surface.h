#ifndef WINDOW_H
#define WINDOW_H

#include <fcg/types.h>

#include "destructor.h"

typedef struct FCG_GDI_s FCG_GDI;
typedef struct FCG_Machine_s FCG_Machine;

typedef enum FCG_ContextType_s
{
    FCG_SURFACE_WINDOW = 0x01
} FCG_ContextType;

typedef struct FCG_ContextSize_s
{
    uint32_t width;
    uint32_t height;
} FCG_ContextSize;

// Maybe implement a FCG_Surface_GetImage
// For regular "framebuffers" this will just yield the stored image
// for a window which contains a swapchain, it'll query the swapchain 
// and return the correct image given the current image index... something
// like that
typedef struct FCG_Image_s
{
    FCG_Handle frame_buffer;
    FCG_Handle image;
    FCG_Handle view;
} FCG_Image;

typedef struct FCG_SurfaceImage_s
{
    U32             index;       // current index (useful?)
    U32             image_count;
    FCG_Handle      handle;      // Swapchain
    FCG_Handle      pass_handle; // render pass handle
    FCG_Image*      images;      // contains each of the images
    U32             format;      // format of all the contained images
    FCG_ContextSize size;        // size of all the contained images
} FCG_SurfaceImage;

typedef struct FCG_Surface_s
{
    FCG_ContextType  type;
    FCG_Handle       handle;
    FCG_Bool         active;
    FCG_Handle       context;
    FCG_ContextSize  size;
    FCG_SurfaceImage surface_image;
    FCG_Memory_Stack destructor_stack;
} FCG_Surface;

typedef struct FCG_WindowData_s
{
    FCG_ContextSize size;
    const char*     title;
} FCG_WindowData;

FCG_Result 
FCG_Surface_Create(
    FCG_Surface* FCG_CR surface, 
    FCG_ContextType type, 
    FCG_DataHandle const data);

/* Have a surface contain its own destructor stack */
/* and push this function to machine's destructor  */
/* stack whenever FCG_Surface_Initialize(...) is   */
/* called. This way, each vulkan object does not   */
/* exist inside a "global scope", but instead only */
/* inside the scope of the objects within which it */
/* was created.                                    */
FCG_Result 
FCG_Surface_Destroy(
    FCG_Surface* FCG_CR surface);

FCG_Result 
FCG_Surface_Initialize(
    FCG_Surface* FCG_CR surface, 
    FCG_Machine* FCG_CR machine,
    const FCG_GDI* FCG_CR instance);

void FCG_DisplaySurface(const FCG_Surface* FCG_CR surface);
void FCG_KeepSurfaceOpen(const FCG_Surface* FCG_CR surface);

#endif