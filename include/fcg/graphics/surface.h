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

typedef struct
{
    FCG_ContextType type;
    FCG_Handle      handle;
    FCG_Bool        active;
    FCG_Handle      context;
    FCG_ContextSize size;
} FCG_Surface;

typedef struct
{
    FCG_ContextSize size;
    const char*     title;
} FCG_WindowData;

FCG_Result FCG_CreateSurface(FCG_Surface* FCG_CR surface, FCG_ContextType type, FCG_DataHandle const data);
FCG_Result FCG_DestroySurface(FCG_Surface* FCG_CR surface);

void FCG_DisplaySurface(const FCG_Surface* FCG_CR surface);
void FCG_KeepSurfaceOpen(const FCG_Surface* FCG_CR surface);

#endif