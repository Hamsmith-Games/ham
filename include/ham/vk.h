/*
 * Ham Runtime
 * Copyright (C) 2022  Hamsmith Ltd.
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

#ifndef HAM_VK_H
#define HAM_VK_H 1

#include "vk_fns.h" // IWYU pragma: keep

HAM_C_API_BEGIN

ham_constexpr ham_nothrow static inline const char *ham_vk_result_str(VkResult result){
	switch(result){
	#define HAM_CASE(val) case (val): return #val;

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

ham_constexpr ham_nothrow const char *ham_vk_color_space_str(VkColorSpaceKHR color_space){
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

ham_constexpr ham_nothrow const char *ham_vk_format_str(VkFormat format){
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

HAM_C_API_END

#endif // !HAM_VK_H
