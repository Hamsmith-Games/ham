#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN 1
#define VK_USE_PLATFORM_WIN32_KHR 1
#endif

#define _GNU_SOURCE 1

#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"
