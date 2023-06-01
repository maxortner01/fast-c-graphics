#ifndef WINDOW_H
#define WINDOW_H

#include <fcg/types.h>

typedef enum
{
    FCG_SURFACE_WINDOW = 0x01
} FCG_ContextType;

typedef struct 
{
    uint32_t width;
    uint32_t height;
} FCG_ContextSize;

// Maybe implement a FCG_Surface_GetImage
// For regular "framebuffers" this will just yield the stored image
// for a window which contains a swapchain, it'll query the swapchain 
// and return the correct image given the current image index... something
// like that
typedef struct
{
    FCG_Handle image;
    FCG_Handle view;
} FCG_Image;

typedef struct
{
    U32        index;
    U32        image_count;
    FCG_Handle handle;
    FCG_Image* images;
} FCG_SurfaceImage;

typedef struct
{
    FCG_ContextType  type;
    FCG_Handle       handle;
    FCG_Bool         active;
    FCG_Handle       context;
    FCG_ContextSize  size;
    FCG_SurfaceImage surface_image;
} FCG_Surface;

typedef struct
{
    FCG_ContextSize size;
    const char*     title;
} FCG_WindowData;

// Possibly change to FCG_Surface_Create/Destroy...
FCG_Result FCG_CreateSurface(FCG_Surface* FCG_CR surface, FCG_ContextType type, FCG_DataHandle const data);
FCG_Result FCG_DestroySurface(FCG_Surface* FCG_CR surface);

void FCG_DisplaySurface(const FCG_Surface* FCG_CR surface);
void FCG_KeepSurfaceOpen(const FCG_Surface* FCG_CR surface);

#endif