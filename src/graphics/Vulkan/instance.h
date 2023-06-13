#ifndef INSTANCE_H_
#define INSTANCE_H_

#include <fcg/graphics/device.h>
#include "../../assert.c"

#include <vulkan/vulkan.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

void 
destroy_instance(
    FCG_Memory_Stack* FCG_CR stack);

void 
destroy_surface(
    FCG_Memory_Stack* FCG_CR stack);

#ifdef FCG_DEBUG

VKAPI_ATTR VkBool32 VKAPI_CALL 
debug_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData);

void 
set_debug_create_info(
    VkDebugUtilsMessengerCreateInfoEXT* create_info);

VkResult
create_vulkan_instance(
    VkInstance* instance);


#ifdef FCG_DEBUG

VkResult
_CreateDebugUtilsMessengerEXT(
    VkInstance instance, 
    const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, 
    const VkAllocationCallbacks* pAllocator, 
    VkDebugUtilsMessengerEXT* pDebugMessenger);

void 
_DestroyDebugUtilsMessengerEXT(
    VkInstance instance, 
    VkDebugUtilsMessengerEXT debugMessenger, 
    const VkAllocationCallbacks* pAllocator);

void destroy_debug_messenger(
    FCG_Memory_Stack* FCG_CR stack);

VkResult 
create_debug_messenger(
    VkInstance                FCG_CR instance,
    VkDebugUtilsMessengerEXT* FCG_CR messenger);

#endif

#endif

#endif