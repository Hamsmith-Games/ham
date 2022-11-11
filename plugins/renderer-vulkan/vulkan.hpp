/*
 * Ham Runtime Plugins
 * Copyright (C) 2022 Keith Hammond
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef HAM_RENDERER_VULKAN_VULKAN_HPP
#define HAM_RENDERER_VULKAN_VULKAN_HPP 1

#define VK_NO_PROTOTYPES 1

#include "ham/log.h"
#include "ham/vk.h"

#include "ham/std_vector.hpp"

#include <array>

namespace ham::vk{
	constexpr static inline const char *result_to_str(VkResult res){
		switch(res){
		#define HAM_CASE(val_) case (val_): return #val_;

			HAM_CASE(VK_SUCCESS)
			HAM_CASE(VK_NOT_READY)
			HAM_CASE(VK_TIMEOUT)
			HAM_CASE(VK_EVENT_SET)
			HAM_CASE(VK_EVENT_RESET)
			HAM_CASE(VK_INCOMPLETE)
			HAM_CASE(VK_ERROR_OUT_OF_HOST_MEMORY)
			HAM_CASE(VK_ERROR_OUT_OF_DEVICE_MEMORY)
			HAM_CASE(VK_ERROR_INITIALIZATION_FAILED)
			HAM_CASE(VK_ERROR_DEVICE_LOST)
			HAM_CASE(VK_ERROR_MEMORY_MAP_FAILED)
			HAM_CASE(VK_ERROR_LAYER_NOT_PRESENT)
			HAM_CASE(VK_ERROR_EXTENSION_NOT_PRESENT)
			HAM_CASE(VK_ERROR_FEATURE_NOT_PRESENT)
			HAM_CASE(VK_ERROR_INCOMPATIBLE_DRIVER)
			HAM_CASE(VK_ERROR_TOO_MANY_OBJECTS)
			HAM_CASE(VK_ERROR_FORMAT_NOT_SUPPORTED)
			HAM_CASE(VK_ERROR_FRAGMENTED_POOL)
			HAM_CASE(VK_ERROR_UNKNOWN)
			HAM_CASE(VK_ERROR_OUT_OF_POOL_MEMORY)
			HAM_CASE(VK_ERROR_INVALID_EXTERNAL_HANDLE)
			HAM_CASE(VK_ERROR_FRAGMENTATION)
			HAM_CASE(VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS)
			HAM_CASE(VK_PIPELINE_COMPILE_REQUIRED)
			HAM_CASE(VK_ERROR_SURFACE_LOST_KHR)
			HAM_CASE(VK_ERROR_NATIVE_WINDOW_IN_USE_KHR)
			HAM_CASE(VK_SUBOPTIMAL_KHR)
			HAM_CASE(VK_ERROR_OUT_OF_DATE_KHR)
			HAM_CASE(VK_ERROR_INCOMPATIBLE_DISPLAY_KHR)
			HAM_CASE(VK_ERROR_VALIDATION_FAILED_EXT)
			HAM_CASE(VK_ERROR_INVALID_SHADER_NV)

		#ifdef VK_ENABLE_BETA_EXTENSIONS
			HAM_CASE(VK_ERROR_IMAGE_USAGE_NOT_SUPPORTED_KHR)
			HAM_CASE(VK_ERROR_VIDEO_PICTURE_LAYOUT_NOT_SUPPORTED_KHR)
			HAM_CASE(VK_ERROR_VIDEO_PROFILE_OPERATION_NOT_SUPPORTED_KHR)
			HAM_CASE(VK_ERROR_VIDEO_PROFILE_FORMAT_NOT_SUPPORTED_KHR)
			HAM_CASE(VK_ERROR_VIDEO_PROFILE_CODEC_NOT_SUPPORTED_KHR)
			HAM_CASE(VK_ERROR_VIDEO_STD_VERSION_NOT_SUPPORTED_KHR)
		#endif

			HAM_CASE(VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT)
			HAM_CASE(VK_ERROR_NOT_PERMITTED_KHR)
			HAM_CASE(VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT)
			HAM_CASE(VK_THREAD_IDLE_KHR)
			HAM_CASE(VK_THREAD_DONE_KHR)
			HAM_CASE(VK_OPERATION_DEFERRED_KHR)
			HAM_CASE(VK_OPERATION_NOT_DEFERRED_KHR)
			HAM_CASE(VK_ERROR_COMPRESSION_EXHAUSTED_EXT)

		#undef HAM_CASE

			default: return "UNKNOWN";
		}
	}

	constexpr static inline const char *format_to_str(VkFormat format){
		switch(format){
		#define HAM_CASE(val) case (val): return #val;

			HAM_CASE(VK_FORMAT_UNDEFINED)
			HAM_CASE(VK_FORMAT_R4G4_UNORM_PACK8)
			HAM_CASE(VK_FORMAT_R4G4B4A4_UNORM_PACK16)
			HAM_CASE(VK_FORMAT_B4G4R4A4_UNORM_PACK16)
			HAM_CASE(VK_FORMAT_R5G6B5_UNORM_PACK16)
			HAM_CASE(VK_FORMAT_B5G6R5_UNORM_PACK16)
			HAM_CASE(VK_FORMAT_R5G5B5A1_UNORM_PACK16)
			HAM_CASE(VK_FORMAT_B5G5R5A1_UNORM_PACK16)
			HAM_CASE(VK_FORMAT_A1R5G5B5_UNORM_PACK16)
			HAM_CASE(VK_FORMAT_R8_UNORM)
			HAM_CASE(VK_FORMAT_R8_SNORM)
			HAM_CASE(VK_FORMAT_R8_USCALED)
			HAM_CASE(VK_FORMAT_R8_SSCALED)
			HAM_CASE(VK_FORMAT_R8_UINT)
			HAM_CASE(VK_FORMAT_R8_SINT)
			HAM_CASE(VK_FORMAT_R8_SRGB)
			HAM_CASE(VK_FORMAT_R8G8_UNORM)
			HAM_CASE(VK_FORMAT_R8G8_SNORM)
			HAM_CASE(VK_FORMAT_R8G8_USCALED)
			HAM_CASE(VK_FORMAT_R8G8_SSCALED)
			HAM_CASE(VK_FORMAT_R8G8_UINT)
			HAM_CASE(VK_FORMAT_R8G8_SINT)
			HAM_CASE(VK_FORMAT_R8G8_SRGB)
			HAM_CASE(VK_FORMAT_R8G8B8_UNORM)
			HAM_CASE(VK_FORMAT_R8G8B8_SNORM)
			HAM_CASE(VK_FORMAT_R8G8B8_USCALED)
			HAM_CASE(VK_FORMAT_R8G8B8_SSCALED)
			HAM_CASE(VK_FORMAT_R8G8B8_UINT)
			HAM_CASE(VK_FORMAT_R8G8B8_SINT)
			HAM_CASE(VK_FORMAT_R8G8B8_SRGB)
			HAM_CASE(VK_FORMAT_B8G8R8_UNORM)
			HAM_CASE(VK_FORMAT_B8G8R8_SNORM)
			HAM_CASE(VK_FORMAT_B8G8R8_USCALED)
			HAM_CASE(VK_FORMAT_B8G8R8_SSCALED)
			HAM_CASE(VK_FORMAT_B8G8R8_UINT)
			HAM_CASE(VK_FORMAT_B8G8R8_SINT)
			HAM_CASE(VK_FORMAT_B8G8R8_SRGB)
			HAM_CASE(VK_FORMAT_R8G8B8A8_UNORM)
			HAM_CASE(VK_FORMAT_R8G8B8A8_SNORM)
			HAM_CASE(VK_FORMAT_R8G8B8A8_USCALED)
			HAM_CASE(VK_FORMAT_R8G8B8A8_SSCALED)
			HAM_CASE(VK_FORMAT_R8G8B8A8_UINT)
			HAM_CASE(VK_FORMAT_R8G8B8A8_SINT)
			HAM_CASE(VK_FORMAT_R8G8B8A8_SRGB)
			HAM_CASE(VK_FORMAT_B8G8R8A8_UNORM)
			HAM_CASE(VK_FORMAT_B8G8R8A8_SNORM)
			HAM_CASE(VK_FORMAT_B8G8R8A8_USCALED)
			HAM_CASE(VK_FORMAT_B8G8R8A8_SSCALED)
			HAM_CASE(VK_FORMAT_B8G8R8A8_UINT)
			HAM_CASE(VK_FORMAT_B8G8R8A8_SINT)
			HAM_CASE(VK_FORMAT_B8G8R8A8_SRGB)
			HAM_CASE(VK_FORMAT_A8B8G8R8_UNORM_PACK32)
			HAM_CASE(VK_FORMAT_A8B8G8R8_SNORM_PACK32)
			HAM_CASE(VK_FORMAT_A8B8G8R8_USCALED_PACK32)
			HAM_CASE(VK_FORMAT_A8B8G8R8_SSCALED_PACK32)
			HAM_CASE(VK_FORMAT_A8B8G8R8_UINT_PACK32)
			HAM_CASE(VK_FORMAT_A8B8G8R8_SINT_PACK32)
			HAM_CASE(VK_FORMAT_A8B8G8R8_SRGB_PACK32)
			HAM_CASE(VK_FORMAT_A2R10G10B10_UNORM_PACK32)
			HAM_CASE(VK_FORMAT_A2R10G10B10_SNORM_PACK32)
			HAM_CASE(VK_FORMAT_A2R10G10B10_USCALED_PACK32)
			HAM_CASE(VK_FORMAT_A2R10G10B10_SSCALED_PACK32)
			HAM_CASE(VK_FORMAT_A2R10G10B10_UINT_PACK32)
			HAM_CASE(VK_FORMAT_A2R10G10B10_SINT_PACK32)
			HAM_CASE(VK_FORMAT_A2B10G10R10_UNORM_PACK32)
			HAM_CASE(VK_FORMAT_A2B10G10R10_SNORM_PACK32)
			HAM_CASE(VK_FORMAT_A2B10G10R10_USCALED_PACK32)
			HAM_CASE(VK_FORMAT_A2B10G10R10_SSCALED_PACK32)
			HAM_CASE(VK_FORMAT_A2B10G10R10_UINT_PACK32)
			HAM_CASE(VK_FORMAT_A2B10G10R10_SINT_PACK32)
			HAM_CASE(VK_FORMAT_R16_UNORM)
			HAM_CASE(VK_FORMAT_R16_SNORM)
			HAM_CASE(VK_FORMAT_R16_USCALED)
			HAM_CASE(VK_FORMAT_R16_SSCALED)
			HAM_CASE(VK_FORMAT_R16_UINT)
			HAM_CASE(VK_FORMAT_R16_SINT)
			HAM_CASE(VK_FORMAT_R16_SFLOAT)
			HAM_CASE(VK_FORMAT_R16G16_UNORM)
			HAM_CASE(VK_FORMAT_R16G16_SNORM)
			HAM_CASE(VK_FORMAT_R16G16_USCALED)
			HAM_CASE(VK_FORMAT_R16G16_SSCALED)
			HAM_CASE(VK_FORMAT_R16G16_UINT)
			HAM_CASE(VK_FORMAT_R16G16_SINT)
			HAM_CASE(VK_FORMAT_R16G16_SFLOAT)
			HAM_CASE(VK_FORMAT_R16G16B16_UNORM)
			HAM_CASE(VK_FORMAT_R16G16B16_SNORM)
			HAM_CASE(VK_FORMAT_R16G16B16_USCALED)
			HAM_CASE(VK_FORMAT_R16G16B16_SSCALED)
			HAM_CASE(VK_FORMAT_R16G16B16_UINT)
			HAM_CASE(VK_FORMAT_R16G16B16_SINT)
			HAM_CASE(VK_FORMAT_R16G16B16_SFLOAT)
			HAM_CASE(VK_FORMAT_R16G16B16A16_UNORM)
			HAM_CASE(VK_FORMAT_R16G16B16A16_SNORM)
			HAM_CASE(VK_FORMAT_R16G16B16A16_USCALED)
			HAM_CASE(VK_FORMAT_R16G16B16A16_SSCALED)
			HAM_CASE(VK_FORMAT_R16G16B16A16_UINT)
			HAM_CASE(VK_FORMAT_R16G16B16A16_SINT)
			HAM_CASE(VK_FORMAT_R16G16B16A16_SFLOAT)
			HAM_CASE(VK_FORMAT_R32_UINT)
			HAM_CASE(VK_FORMAT_R32_SINT)
			HAM_CASE(VK_FORMAT_R32_SFLOAT)
			HAM_CASE(VK_FORMAT_R32G32_UINT)
			HAM_CASE(VK_FORMAT_R32G32_SINT)
			HAM_CASE(VK_FORMAT_R32G32_SFLOAT)
			HAM_CASE(VK_FORMAT_R32G32B32_UINT)
			HAM_CASE(VK_FORMAT_R32G32B32_SINT)
			HAM_CASE(VK_FORMAT_R32G32B32_SFLOAT)
			HAM_CASE(VK_FORMAT_R32G32B32A32_UINT)
			HAM_CASE(VK_FORMAT_R32G32B32A32_SINT)
			HAM_CASE(VK_FORMAT_R32G32B32A32_SFLOAT)
			HAM_CASE(VK_FORMAT_R64_UINT)
			HAM_CASE(VK_FORMAT_R64_SINT)
			HAM_CASE(VK_FORMAT_R64_SFLOAT)
			HAM_CASE(VK_FORMAT_R64G64_UINT)
			HAM_CASE(VK_FORMAT_R64G64_SINT)
			HAM_CASE(VK_FORMAT_R64G64_SFLOAT)
			HAM_CASE(VK_FORMAT_R64G64B64_UINT)
			HAM_CASE(VK_FORMAT_R64G64B64_SINT)
			HAM_CASE(VK_FORMAT_R64G64B64_SFLOAT)
			HAM_CASE(VK_FORMAT_R64G64B64A64_UINT)
			HAM_CASE(VK_FORMAT_R64G64B64A64_SINT)
			HAM_CASE(VK_FORMAT_R64G64B64A64_SFLOAT)
			HAM_CASE(VK_FORMAT_B10G11R11_UFLOAT_PACK32)
			HAM_CASE(VK_FORMAT_E5B9G9R9_UFLOAT_PACK32)
			HAM_CASE(VK_FORMAT_D16_UNORM)
			HAM_CASE(VK_FORMAT_X8_D24_UNORM_PACK32)
			HAM_CASE(VK_FORMAT_D32_SFLOAT)
			HAM_CASE(VK_FORMAT_S8_UINT)
			HAM_CASE(VK_FORMAT_D16_UNORM_S8_UINT)
			HAM_CASE(VK_FORMAT_D24_UNORM_S8_UINT)
			HAM_CASE(VK_FORMAT_D32_SFLOAT_S8_UINT)
			HAM_CASE(VK_FORMAT_BC1_RGB_UNORM_BLOCK)
			HAM_CASE(VK_FORMAT_BC1_RGB_SRGB_BLOCK)
			HAM_CASE(VK_FORMAT_BC1_RGBA_UNORM_BLOCK)
			HAM_CASE(VK_FORMAT_BC1_RGBA_SRGB_BLOCK)
			HAM_CASE(VK_FORMAT_BC2_UNORM_BLOCK)
			HAM_CASE(VK_FORMAT_BC2_SRGB_BLOCK)
			HAM_CASE(VK_FORMAT_BC3_UNORM_BLOCK)
			HAM_CASE(VK_FORMAT_BC3_SRGB_BLOCK)
			HAM_CASE(VK_FORMAT_BC4_UNORM_BLOCK)
			HAM_CASE(VK_FORMAT_BC4_SNORM_BLOCK)
			HAM_CASE(VK_FORMAT_BC5_UNORM_BLOCK)
			HAM_CASE(VK_FORMAT_BC5_SNORM_BLOCK)
			HAM_CASE(VK_FORMAT_BC6H_UFLOAT_BLOCK)
			HAM_CASE(VK_FORMAT_BC6H_SFLOAT_BLOCK)
			HAM_CASE(VK_FORMAT_BC7_UNORM_BLOCK)
			HAM_CASE(VK_FORMAT_BC7_SRGB_BLOCK)
			HAM_CASE(VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK)
			HAM_CASE(VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK)
			HAM_CASE(VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK)
			HAM_CASE(VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK)
			HAM_CASE(VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK)
			HAM_CASE(VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK)
			HAM_CASE(VK_FORMAT_EAC_R11_UNORM_BLOCK)
			HAM_CASE(VK_FORMAT_EAC_R11_SNORM_BLOCK)
			HAM_CASE(VK_FORMAT_EAC_R11G11_UNORM_BLOCK)
			HAM_CASE(VK_FORMAT_EAC_R11G11_SNORM_BLOCK)
			HAM_CASE(VK_FORMAT_ASTC_4x4_UNORM_BLOCK)
			HAM_CASE(VK_FORMAT_ASTC_4x4_SRGB_BLOCK)
			HAM_CASE(VK_FORMAT_ASTC_5x4_UNORM_BLOCK)
			HAM_CASE(VK_FORMAT_ASTC_5x4_SRGB_BLOCK)
			HAM_CASE(VK_FORMAT_ASTC_5x5_UNORM_BLOCK)
			HAM_CASE(VK_FORMAT_ASTC_5x5_SRGB_BLOCK)
			HAM_CASE(VK_FORMAT_ASTC_6x5_UNORM_BLOCK)
			HAM_CASE(VK_FORMAT_ASTC_6x5_SRGB_BLOCK)
			HAM_CASE(VK_FORMAT_ASTC_6x6_UNORM_BLOCK)
			HAM_CASE(VK_FORMAT_ASTC_6x6_SRGB_BLOCK)
			HAM_CASE(VK_FORMAT_ASTC_8x5_UNORM_BLOCK)
			HAM_CASE(VK_FORMAT_ASTC_8x5_SRGB_BLOCK)
			HAM_CASE(VK_FORMAT_ASTC_8x6_UNORM_BLOCK)
			HAM_CASE(VK_FORMAT_ASTC_8x6_SRGB_BLOCK)
			HAM_CASE(VK_FORMAT_ASTC_8x8_UNORM_BLOCK)
			HAM_CASE(VK_FORMAT_ASTC_8x8_SRGB_BLOCK)
			HAM_CASE(VK_FORMAT_ASTC_10x5_UNORM_BLOCK)
			HAM_CASE(VK_FORMAT_ASTC_10x5_SRGB_BLOCK)
			HAM_CASE(VK_FORMAT_ASTC_10x6_UNORM_BLOCK)
			HAM_CASE(VK_FORMAT_ASTC_10x6_SRGB_BLOCK)
			HAM_CASE(VK_FORMAT_ASTC_10x8_UNORM_BLOCK)
			HAM_CASE(VK_FORMAT_ASTC_10x8_SRGB_BLOCK)
			HAM_CASE(VK_FORMAT_ASTC_10x10_UNORM_BLOCK)
			HAM_CASE(VK_FORMAT_ASTC_10x10_SRGB_BLOCK)
			HAM_CASE(VK_FORMAT_ASTC_12x10_UNORM_BLOCK)
			HAM_CASE(VK_FORMAT_ASTC_12x10_SRGB_BLOCK)
			HAM_CASE(VK_FORMAT_ASTC_12x12_UNORM_BLOCK)
			HAM_CASE(VK_FORMAT_ASTC_12x12_SRGB_BLOCK)
			HAM_CASE(VK_FORMAT_G8B8G8R8_422_UNORM)
			HAM_CASE(VK_FORMAT_B8G8R8G8_422_UNORM)
			HAM_CASE(VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM)
			HAM_CASE(VK_FORMAT_G8_B8R8_2PLANE_420_UNORM)
			HAM_CASE(VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM)
			HAM_CASE(VK_FORMAT_G8_B8R8_2PLANE_422_UNORM)
			HAM_CASE(VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM)
			HAM_CASE(VK_FORMAT_R10X6_UNORM_PACK16)
			HAM_CASE(VK_FORMAT_R10X6G10X6_UNORM_2PACK16)
			HAM_CASE(VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16)
			HAM_CASE(VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16)
			HAM_CASE(VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16)
			HAM_CASE(VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16)
			HAM_CASE(VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16)
			HAM_CASE(VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16)
			HAM_CASE(VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16)
			HAM_CASE(VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16)
			HAM_CASE(VK_FORMAT_R12X4_UNORM_PACK16)
			HAM_CASE(VK_FORMAT_R12X4G12X4_UNORM_2PACK16)
			HAM_CASE(VK_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16)
			HAM_CASE(VK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16)
			HAM_CASE(VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16)
			HAM_CASE(VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16)
			HAM_CASE(VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16)
			HAM_CASE(VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16)
			HAM_CASE(VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16)
			HAM_CASE(VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16)
			HAM_CASE(VK_FORMAT_G16B16G16R16_422_UNORM)
			HAM_CASE(VK_FORMAT_B16G16R16G16_422_UNORM)
			HAM_CASE(VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM)
			HAM_CASE(VK_FORMAT_G16_B16R16_2PLANE_420_UNORM)
			HAM_CASE(VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM)
			HAM_CASE(VK_FORMAT_G16_B16R16_2PLANE_422_UNORM)
			HAM_CASE(VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM)
			HAM_CASE(VK_FORMAT_G8_B8R8_2PLANE_444_UNORM)
			HAM_CASE(VK_FORMAT_G10X6_B10X6R10X6_2PLANE_444_UNORM_3PACK16)
			HAM_CASE(VK_FORMAT_G12X4_B12X4R12X4_2PLANE_444_UNORM_3PACK16)
			HAM_CASE(VK_FORMAT_G16_B16R16_2PLANE_444_UNORM)
			HAM_CASE(VK_FORMAT_A4R4G4B4_UNORM_PACK16)
			HAM_CASE(VK_FORMAT_A4B4G4R4_UNORM_PACK16)
			HAM_CASE(VK_FORMAT_ASTC_4x4_SFLOAT_BLOCK)
			HAM_CASE(VK_FORMAT_ASTC_5x4_SFLOAT_BLOCK)
			HAM_CASE(VK_FORMAT_ASTC_5x5_SFLOAT_BLOCK)
			HAM_CASE(VK_FORMAT_ASTC_6x5_SFLOAT_BLOCK)
			HAM_CASE(VK_FORMAT_ASTC_6x6_SFLOAT_BLOCK)
			HAM_CASE(VK_FORMAT_ASTC_8x5_SFLOAT_BLOCK)
			HAM_CASE(VK_FORMAT_ASTC_8x6_SFLOAT_BLOCK)
			HAM_CASE(VK_FORMAT_ASTC_8x8_SFLOAT_BLOCK)
			HAM_CASE(VK_FORMAT_ASTC_10x5_SFLOAT_BLOCK)
			HAM_CASE(VK_FORMAT_ASTC_10x6_SFLOAT_BLOCK)
			HAM_CASE(VK_FORMAT_ASTC_10x8_SFLOAT_BLOCK)
			HAM_CASE(VK_FORMAT_ASTC_10x10_SFLOAT_BLOCK)
			HAM_CASE(VK_FORMAT_ASTC_12x10_SFLOAT_BLOCK)
			HAM_CASE(VK_FORMAT_ASTC_12x12_SFLOAT_BLOCK)
			HAM_CASE(VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG)
			HAM_CASE(VK_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG)
			HAM_CASE(VK_FORMAT_PVRTC2_2BPP_UNORM_BLOCK_IMG)
			HAM_CASE(VK_FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG)
			HAM_CASE(VK_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG)
			HAM_CASE(VK_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG)
			HAM_CASE(VK_FORMAT_PVRTC2_2BPP_SRGB_BLOCK_IMG)
			HAM_CASE(VK_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG)

		#undef HAM_CASE

			default: return "UNKNOWN";
		}
	}

	constexpr static inline const char *color_space_to_str(VkColorSpaceKHR color_space){
		switch(color_space){
		#define HAM_CASE(val) case (val): return #val;

			HAM_CASE(VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			HAM_CASE(VK_COLOR_SPACE_DISPLAY_P3_NONLINEAR_EXT)
			HAM_CASE(VK_COLOR_SPACE_EXTENDED_SRGB_LINEAR_EXT)
			HAM_CASE(VK_COLOR_SPACE_DISPLAY_P3_LINEAR_EXT)
			HAM_CASE(VK_COLOR_SPACE_DCI_P3_NONLINEAR_EXT)
			HAM_CASE(VK_COLOR_SPACE_BT709_LINEAR_EXT)
			HAM_CASE(VK_COLOR_SPACE_BT709_NONLINEAR_EXT)
			HAM_CASE(VK_COLOR_SPACE_BT2020_LINEAR_EXT)
			HAM_CASE(VK_COLOR_SPACE_HDR10_ST2084_EXT)
			HAM_CASE(VK_COLOR_SPACE_DOLBYVISION_EXT)
			HAM_CASE(VK_COLOR_SPACE_HDR10_HLG_EXT)
			HAM_CASE(VK_COLOR_SPACE_ADOBERGB_LINEAR_EXT)
			HAM_CASE(VK_COLOR_SPACE_ADOBERGB_NONLINEAR_EXT)
			HAM_CASE(VK_COLOR_SPACE_PASS_THROUGH_EXT)
			HAM_CASE(VK_COLOR_SPACE_EXTENDED_SRGB_NONLINEAR_EXT)
			HAM_CASE(VK_COLOR_SPACE_DISPLAY_NATIVE_AMD)

		#undef HAM_CASE

			default: return "UNKNOWN";
		}
	}

	struct swapchain_info{
		VkSurfaceCapabilitiesKHR capabilities;
		ham::std_vector<VkSurfaceFormatKHR> surface_formats;
		ham::std_vector<VkPresentModeKHR> present_modes;
	};

	class result{
		public:
			result(VkResult val) noexcept
				: m_val(val){}

			result &operator=(VkResult val) noexcept{
				m_val = val;
				return *this;
			}

			operator VkResult() const noexcept{ return m_val; }

			bool operator==(VkResult other) const noexcept{ return m_val == other; }
			bool operator!=(VkResult other) const noexcept{ return m_val != other; }

			const char *to_str() const noexcept{ return result_to_str(m_val); }

			VkResult m_val;
	};

	namespace detail{
		template<typename CreateInfo>
		class create_info_wrapper{
			public:
				template<typename ... Args>
				create_info_wrapper(VkStructureType s_type, const void *p_next, Args &&... args) noexcept
					: m_val{ s_type, p_next, std::forward<Args>(args)... }{}

				operator const CreateInfo&() const noexcept{ return m_val; }
				operator CreateInfo&() & noexcept{ return m_val; }

				CreateInfo *ptr() noexcept{ return &m_val; }
				const CreateInfo *ptr() const noexcept{ return &m_val; }

				CreateInfo *operator->() & noexcept{ return &m_val; }
				const CreateInfo *operator->() const& noexcept{ return &m_val; }

				CreateInfo m_val;
		};
	}

	class pipeline_shader_stage_create_info: public detail::create_info_wrapper<VkPipelineShaderStageCreateInfo>{
		public:
			pipeline_shader_stage_create_info(
				const void *p_next,
				VkPipelineShaderStageCreateFlags flags,
				VkShaderStageFlagBits stage,
				VkShaderModule module,
				const char *name,
				const VkSpecializationInfo *spec_info
			) noexcept
				: create_info_wrapper(
					VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, p_next,
					flags, stage, module, name, spec_info
				){}
	};

	class pipeline_dynamic_state_create_info: public detail::create_info_wrapper<VkPipelineDynamicStateCreateInfo>{
		public:
			pipeline_dynamic_state_create_info(
				const void *p_next,
				VkPipelineDynamicStateCreateFlags flags,
				u32 num_dynamic_states,
				const VkDynamicState *p_dynamic_states
			)
				: create_info_wrapper(
					VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO, p_next,
					flags, num_dynamic_states, p_dynamic_states
				){}
	};

	class pipeline_vertex_input_state_create_info: public detail::create_info_wrapper<VkPipelineVertexInputStateCreateInfo>{
		public:
			pipeline_vertex_input_state_create_info(
				const void *p_next,
				VkPipelineVertexInputStateCreateFlags flags,
				u32 num_vertex_binding_descriptions,
				const VkVertexInputBindingDescription *p_vertex_binding_descriptions,
				u32 num_vertex_attrib_descriptions,
				const VkVertexInputAttributeDescription *p_vertex_attrib_descriptions
			) noexcept
				: create_info_wrapper(
					VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO, p_next,
					flags, num_vertex_binding_descriptions, p_vertex_binding_descriptions,
					num_vertex_attrib_descriptions, p_vertex_attrib_descriptions
				){}
	};

	class pipeline_input_assembly_state_create_info: public detail::create_info_wrapper<VkPipelineInputAssemblyStateCreateInfo>{
		public:
			pipeline_input_assembly_state_create_info(
				const void *p_next,
				VkPipelineInputAssemblyStateCreateFlags flags,
				VkPrimitiveTopology topology,
				VkBool32 primitive_restart_enable
			) noexcept
				: create_info_wrapper(
					  VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO, p_next,
					  flags, topology, primitive_restart_enable
				){}
	};

	class pipeline_viewport_state_create_info: public detail::create_info_wrapper<VkPipelineViewportStateCreateInfo>{
		public:
			pipeline_viewport_state_create_info(
				const void *p_next,
				VkPipelineViewportStateCreateFlags flags,
				u32 num_viewports,
				const VkViewport *p_viewports,
				u32 num_scissors,
				const VkRect2D *p_scissors
			) noexcept
				: create_info_wrapper(
					VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO, p_next,
					flags, num_viewports, p_viewports, num_scissors, p_scissors
				){}
	};

	class pipeline_rasterization_state_create_info: public detail::create_info_wrapper<VkPipelineRasterizationStateCreateInfo>{
		public:
			pipeline_rasterization_state_create_info(
				const void *p_next,
				VkPipelineRasterizationStateCreateFlags flags,
				VkBool32 depth_clamp_enable,
				VkBool32 rasterizer_discard_enable,
				VkPolygonMode polygon_mode,
				VkCullModeFlags cull_mode,
				VkFrontFace front_face,
				VkBool32 depth_bias_enable,
				float depth_bias_constant_factor,
				float depth_bias_clamp,
				float depth_bias_slope_factor,
				float line_width
			) noexcept
				: create_info_wrapper(
					VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO, p_next,
					flags, depth_clamp_enable, rasterizer_discard_enable, polygon_mode, cull_mode, front_face,
					depth_bias_enable, depth_bias_constant_factor, depth_bias_clamp, depth_bias_slope_factor, line_width
				){}
	};

	class pipeline_multisample_state_create_info: public detail::create_info_wrapper<VkPipelineMultisampleStateCreateInfo>{
		public:
			pipeline_multisample_state_create_info(
				const void *p_next,
				VkPipelineMultisampleStateCreateFlags flags,
				VkSampleCountFlagBits rasterization_samples,
				VkBool32 sample_shading_enable,
				float min_sample_shading,
				const VkSampleMask *p_sample_mask,
				VkBool32 alpha_to_coverage_enable,
				VkBool32 alpha_to_one_enable
			) noexcept
				: create_info_wrapper(
					VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO, p_next,
					flags, rasterization_samples,
					sample_shading_enable, min_sample_shading, p_sample_mask,
					alpha_to_coverage_enable, alpha_to_one_enable
				){}
	};

	class pipeline_color_blend_state_create_info: public detail::create_info_wrapper<VkPipelineColorBlendStateCreateInfo>{
		public:
			pipeline_color_blend_state_create_info(
				const void *p_next,
				VkPipelineColorBlendStateCreateFlags flags,
				VkBool32 logic_op_enable,
				VkLogicOp logic_op,
				uint32_t attachment_count,
				const VkPipelineColorBlendAttachmentState *p_attachments,
				const std::array<float, 4> &blend_constants
			) noexcept
				: create_info_wrapper(
					VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO, p_next,
					flags, logic_op_enable, logic_op, attachment_count, p_attachments
				)
			{
				std::copy(begin(blend_constants), end(blend_constants), m_val.blendConstants);
			}
	};

	class pipeline_layout_create_info: public detail::create_info_wrapper<VkPipelineLayoutCreateInfo>{
		public:
			pipeline_layout_create_info(
				const void *p_next,
				VkPipelineLayoutCreateFlags flags,
				u32 set_layout_count,
				const VkDescriptorSetLayout *p_set_layouts,
				u32 num_push_constant_ranges,
				const VkPushConstantRange *p_push_constant_ranges
			) noexcept
				: create_info_wrapper(
					VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO, p_next, flags,
					set_layout_count, p_set_layouts, num_push_constant_ranges, p_push_constant_ranges
				){}
	};

	class render_pass_create_info: public detail::create_info_wrapper<VkRenderPassCreateInfo>{
		public:
			render_pass_create_info(
				const void *p_next,
				VkRenderPassCreateFlags flags,
				u32 num_attachments,
				const VkAttachmentDescription *p_attachments,
				u32 num_subpasses,
				const VkSubpassDescription *p_subpasses,
				u32 num_dependencies,
				const VkSubpassDependency *p_dependencies
			) noexcept
				: create_info_wrapper(
					VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO, p_next, flags,
					num_attachments, p_attachments,
					num_subpasses, p_subpasses,
					num_dependencies, p_dependencies
				)
			{}
	};

	class graphics_pipeline_create_info: public detail::create_info_wrapper<VkGraphicsPipelineCreateInfo>{
		public:
			graphics_pipeline_create_info(
				const void *p_next,
				VkPipelineCreateFlags flags,
				u32 num_stages,
				const VkPipelineShaderStageCreateInfo *p_stages,
				const VkPipelineVertexInputStateCreateInfo *p_vertex_input_state,
				const VkPipelineInputAssemblyStateCreateInfo *p_input_assembly_state,
				const VkPipelineTessellationStateCreateInfo *p_tessellation_state,
				const VkPipelineViewportStateCreateInfo *p_viewport_state,
				const VkPipelineRasterizationStateCreateInfo *p_rasterization_state,
				const VkPipelineMultisampleStateCreateInfo *p_multisample_state,
				const VkPipelineDepthStencilStateCreateInfo *p_depth_stencil_state,
				const VkPipelineColorBlendStateCreateInfo *p_color_blend_state,
				const VkPipelineDynamicStateCreateInfo *p_dynamic_state,
				VkPipelineLayout layout,
				VkRenderPass render_pass,
				u32 subpass,
				VkPipeline base_pipeline_handle,
				i32 base_pipeline_index
			) noexcept
				: create_info_wrapper(
					VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO, p_next, flags,
					num_stages, p_stages,
					p_vertex_input_state,
					p_input_assembly_state,
					p_tessellation_state,
					p_viewport_state,
					p_rasterization_state,
					p_multisample_state,
					p_depth_stencil_state,
					p_color_blend_state,
					p_dynamic_state,
					layout, render_pass, subpass, base_pipeline_handle, base_pipeline_index
				){}
	};

	class framebuffer_create_info: public detail::create_info_wrapper<VkFramebufferCreateInfo>{
		public:
			framebuffer_create_info(
				const void *p_next,
				VkFramebufferCreateFlags flags,
				VkRenderPass render_pass,
				u32 num_attachments,
				const VkImageView* p_attachments,
				u32 width,
				u32 height,
				u32 layers
			) noexcept
				: create_info_wrapper(
					VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO, p_next, flags,
					render_pass, num_attachments, p_attachments,
					width, height, layers
				){}
	};

	class command_pool_create_info: public detail::create_info_wrapper<VkCommandPoolCreateInfo>{
		public:
			command_pool_create_info(const void *p_next, VkCommandPoolCreateFlags flags, u32 queue_family_idx) noexcept
				: create_info_wrapper(VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO, p_next, flags, queue_family_idx)
			{}
	};

	ham_nonnull_args(2, 3, 4)
	static inline bool get_swapchain_info(const ham_vk_fns &fns, VkPhysicalDevice phys_dev, VkSurfaceKHR surface, swapchain_info *ret){
		result res = fns.vkGetPhysicalDeviceSurfaceCapabilitiesKHR(phys_dev, surface, &ret->capabilities);
		if(res != VK_SUCCESS){
			ham_logerrorf("ham::vk::get_swapchain_infos", "Error in vkGetPhysicalDeviceSurfaceCapabilitiesKHR: %s", res.to_str());
			return false;
		}

		u32 num_formats = 0;
		res = fns.vkGetPhysicalDeviceSurfaceFormatsKHR(phys_dev, surface, &num_formats, nullptr);
		if(res != VK_SUCCESS){
			ham_logerrorf("ham::vk::get_swapchain_infos", "Error in vkGetPhysicalDeviceSurfaceFormatsKHR: %s", res.to_str());
			return false;
		}
		else if(num_formats == 0){
			ham_logerrorf("ham::vk::get_swapchain_infos", "No device surface formats found");
			return false;
		}

		ret->surface_formats.resize(num_formats);

		res = fns.vkGetPhysicalDeviceSurfaceFormatsKHR(phys_dev, surface, &num_formats, ret->surface_formats.data());
		if(res != VK_SUCCESS){
			ham_logerrorf("ham::vk::get_swapchain_infos", "Error in vkGetPhysicalDeviceSurfaceFormatsKHR: %s", res.to_str());
			return false;
		}

		u32 num_modes = 0;
		res = fns.vkGetPhysicalDeviceSurfacePresentModesKHR(phys_dev, surface, &num_modes, nullptr);
		if(res != VK_SUCCESS){
			ham_logerrorf("ham::vk::get_swapchain_infos", "Error in vkGetPhysicalDeviceSurfacePresentModesKHR: %s", res.to_str());
			return false;
		}
		else if(num_modes == 0){
			ham_logerrorf("ham::vk::get_swapchain_infos", "No device surface present modes found");
			return false;
		}

		ret->present_modes.resize(num_modes);

		res = fns.vkGetPhysicalDeviceSurfacePresentModesKHR(phys_dev, surface, &num_modes, ret->present_modes.data());
		if(res != VK_SUCCESS){
			ham_logerrorf("ham::vk::get_swapchain_infos", "Error in vkGetPhysicalDeviceSurfacePresentModesKHR: %s", res.to_str());
			return false;
		}

		return true;
	}

	static inline VkShaderModule create_shader_module(const ham_vk_fns &fs, VkDevice dev, usize buf_size, void *buf){
		VkShaderModuleCreateInfo create_info{ VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO, nullptr, 0, buf_size, (const u32*)buf };

		VkShaderModule ret;
		const auto res = fs.vkCreateShaderModule(dev, &create_info, nullptr, &ret);
		if(res != VK_SUCCESS){
			ham_logapierrorf("Error in vkCreateShaderModule: %s", result_to_str(res));
			return nullptr;
		}

		return ret;
	}

	static inline void destroy_shader_module(const ham_vk_fns &fns, VkDevice dev, VkShaderModule module){
		fns.vkDestroyShaderModule(dev, module, nullptr);
	}

	static inline VkPipelineLayout create_pipeline_layout(const ham_vk_fns &fs, VkDevice dev, const VkPipelineLayoutCreateInfo &create_info){
		VkPipelineLayout ret;
		const auto res = fs.vkCreatePipelineLayout(dev, &create_info, nullptr, &ret);
		if(res != VK_SUCCESS){
			ham_logapierrorf("Error in vkCreatePipelineLayout: %s", result_to_str(res));
			return nullptr;
		}

		return ret;
	}

	static inline void destroy_pipeline_layout(const ham_vk_fns &fs, VkDevice dev, VkPipelineLayout layout){
		fs.vkDestroyPipelineLayout(dev, layout, nullptr);
	}
}

#endif // !HAM_RENDERER_VULKAN_VULKAN_HPP
