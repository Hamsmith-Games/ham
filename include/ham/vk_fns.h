// Generated by ham/utils/gen-vk.py

#ifndef HAM_VK_FNS_H
#define HAM_VK_FNS_H 1

#include "ham/config.h"

#define VK_NO_PROTOTYPES 1
#include <vulkan/vulkan_core.h>

#ifdef HAM_VK_WAYLAND
#	include <vulkan/vulkan_wayland.h>
#endif

#ifdef HAM_VK_XCB
#	include <xcb/xcb.h>
#	include <vulkan/vulkan_xcb.h>
#endif

#ifdef HAM_VK_XLIB
#	include <X11/Xlib.h>
#	include <vulkan/vulkan_xlib.h>
#endif

#ifdef HAM_VK_WIN32
#	include <vulkan/vulkan_win32.h>
#endif

HAM_C_API_BEGIN

typedef struct ham_vk_fns{
	// Vulkan 1.0
	PFN_vkCreateInstance vkCreateInstance;
	PFN_vkDestroyInstance vkDestroyInstance;
	PFN_vkEnumeratePhysicalDevices vkEnumeratePhysicalDevices;
	PFN_vkGetPhysicalDeviceFeatures vkGetPhysicalDeviceFeatures;
	PFN_vkGetPhysicalDeviceFormatProperties vkGetPhysicalDeviceFormatProperties;
	PFN_vkGetPhysicalDeviceImageFormatProperties vkGetPhysicalDeviceImageFormatProperties;
	PFN_vkGetPhysicalDeviceProperties vkGetPhysicalDeviceProperties;
	PFN_vkGetPhysicalDeviceQueueFamilyProperties vkGetPhysicalDeviceQueueFamilyProperties;
	PFN_vkGetPhysicalDeviceMemoryProperties vkGetPhysicalDeviceMemoryProperties;
	PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr;
	PFN_vkGetDeviceProcAddr vkGetDeviceProcAddr;
	PFN_vkCreateDevice vkCreateDevice;
	PFN_vkDestroyDevice vkDestroyDevice;
	PFN_vkEnumerateInstanceExtensionProperties vkEnumerateInstanceExtensionProperties;
	PFN_vkEnumerateDeviceExtensionProperties vkEnumerateDeviceExtensionProperties;
	PFN_vkEnumerateInstanceLayerProperties vkEnumerateInstanceLayerProperties;
	PFN_vkEnumerateDeviceLayerProperties vkEnumerateDeviceLayerProperties;
	PFN_vkGetDeviceQueue vkGetDeviceQueue;
	PFN_vkQueueSubmit vkQueueSubmit;
	PFN_vkQueueWaitIdle vkQueueWaitIdle;
	PFN_vkDeviceWaitIdle vkDeviceWaitIdle;
	PFN_vkAllocateMemory vkAllocateMemory;
	PFN_vkFreeMemory vkFreeMemory;
	PFN_vkMapMemory vkMapMemory;
	PFN_vkUnmapMemory vkUnmapMemory;
	PFN_vkFlushMappedMemoryRanges vkFlushMappedMemoryRanges;
	PFN_vkInvalidateMappedMemoryRanges vkInvalidateMappedMemoryRanges;
	PFN_vkGetDeviceMemoryCommitment vkGetDeviceMemoryCommitment;
	PFN_vkBindBufferMemory vkBindBufferMemory;
	PFN_vkBindImageMemory vkBindImageMemory;
	PFN_vkGetBufferMemoryRequirements vkGetBufferMemoryRequirements;
	PFN_vkGetImageMemoryRequirements vkGetImageMemoryRequirements;
	PFN_vkGetImageSparseMemoryRequirements vkGetImageSparseMemoryRequirements;
	PFN_vkGetPhysicalDeviceSparseImageFormatProperties vkGetPhysicalDeviceSparseImageFormatProperties;
	PFN_vkQueueBindSparse vkQueueBindSparse;
	PFN_vkCreateFence vkCreateFence;
	PFN_vkDestroyFence vkDestroyFence;
	PFN_vkResetFences vkResetFences;
	PFN_vkGetFenceStatus vkGetFenceStatus;
	PFN_vkWaitForFences vkWaitForFences;
	PFN_vkCreateSemaphore vkCreateSemaphore;
	PFN_vkDestroySemaphore vkDestroySemaphore;
	PFN_vkCreateEvent vkCreateEvent;
	PFN_vkDestroyEvent vkDestroyEvent;
	PFN_vkGetEventStatus vkGetEventStatus;
	PFN_vkSetEvent vkSetEvent;
	PFN_vkResetEvent vkResetEvent;
	PFN_vkCreateQueryPool vkCreateQueryPool;
	PFN_vkDestroyQueryPool vkDestroyQueryPool;
	PFN_vkGetQueryPoolResults vkGetQueryPoolResults;
	PFN_vkCreateBuffer vkCreateBuffer;
	PFN_vkDestroyBuffer vkDestroyBuffer;
	PFN_vkCreateBufferView vkCreateBufferView;
	PFN_vkDestroyBufferView vkDestroyBufferView;
	PFN_vkCreateImage vkCreateImage;
	PFN_vkDestroyImage vkDestroyImage;
	PFN_vkGetImageSubresourceLayout vkGetImageSubresourceLayout;
	PFN_vkCreateImageView vkCreateImageView;
	PFN_vkDestroyImageView vkDestroyImageView;
	PFN_vkCreateShaderModule vkCreateShaderModule;
	PFN_vkDestroyShaderModule vkDestroyShaderModule;
	PFN_vkCreatePipelineCache vkCreatePipelineCache;
	PFN_vkDestroyPipelineCache vkDestroyPipelineCache;
	PFN_vkGetPipelineCacheData vkGetPipelineCacheData;
	PFN_vkMergePipelineCaches vkMergePipelineCaches;
	PFN_vkCreateGraphicsPipelines vkCreateGraphicsPipelines;
	PFN_vkCreateComputePipelines vkCreateComputePipelines;
	PFN_vkDestroyPipeline vkDestroyPipeline;
	PFN_vkCreatePipelineLayout vkCreatePipelineLayout;
	PFN_vkDestroyPipelineLayout vkDestroyPipelineLayout;
	PFN_vkCreateSampler vkCreateSampler;
	PFN_vkDestroySampler vkDestroySampler;
	PFN_vkCreateDescriptorSetLayout vkCreateDescriptorSetLayout;
	PFN_vkDestroyDescriptorSetLayout vkDestroyDescriptorSetLayout;
	PFN_vkCreateDescriptorPool vkCreateDescriptorPool;
	PFN_vkDestroyDescriptorPool vkDestroyDescriptorPool;
	PFN_vkResetDescriptorPool vkResetDescriptorPool;
	PFN_vkAllocateDescriptorSets vkAllocateDescriptorSets;
	PFN_vkFreeDescriptorSets vkFreeDescriptorSets;
	PFN_vkUpdateDescriptorSets vkUpdateDescriptorSets;
	PFN_vkCreateFramebuffer vkCreateFramebuffer;
	PFN_vkDestroyFramebuffer vkDestroyFramebuffer;
	PFN_vkCreateRenderPass vkCreateRenderPass;
	PFN_vkDestroyRenderPass vkDestroyRenderPass;
	PFN_vkGetRenderAreaGranularity vkGetRenderAreaGranularity;
	PFN_vkCreateCommandPool vkCreateCommandPool;
	PFN_vkDestroyCommandPool vkDestroyCommandPool;
	PFN_vkResetCommandPool vkResetCommandPool;
	PFN_vkAllocateCommandBuffers vkAllocateCommandBuffers;
	PFN_vkFreeCommandBuffers vkFreeCommandBuffers;
	PFN_vkBeginCommandBuffer vkBeginCommandBuffer;
	PFN_vkEndCommandBuffer vkEndCommandBuffer;
	PFN_vkResetCommandBuffer vkResetCommandBuffer;
	PFN_vkCmdBindPipeline vkCmdBindPipeline;
	PFN_vkCmdSetViewport vkCmdSetViewport;
	PFN_vkCmdSetScissor vkCmdSetScissor;
	PFN_vkCmdSetLineWidth vkCmdSetLineWidth;
	PFN_vkCmdSetDepthBias vkCmdSetDepthBias;
	PFN_vkCmdSetBlendConstants vkCmdSetBlendConstants;
	PFN_vkCmdSetDepthBounds vkCmdSetDepthBounds;
	PFN_vkCmdSetStencilCompareMask vkCmdSetStencilCompareMask;
	PFN_vkCmdSetStencilWriteMask vkCmdSetStencilWriteMask;
	PFN_vkCmdSetStencilReference vkCmdSetStencilReference;
	PFN_vkCmdBindDescriptorSets vkCmdBindDescriptorSets;
	PFN_vkCmdBindIndexBuffer vkCmdBindIndexBuffer;
	PFN_vkCmdBindVertexBuffers vkCmdBindVertexBuffers;
	PFN_vkCmdDraw vkCmdDraw;
	PFN_vkCmdDrawIndexed vkCmdDrawIndexed;
	PFN_vkCmdDrawIndirect vkCmdDrawIndirect;
	PFN_vkCmdDrawIndexedIndirect vkCmdDrawIndexedIndirect;
	PFN_vkCmdDispatch vkCmdDispatch;
	PFN_vkCmdDispatchIndirect vkCmdDispatchIndirect;
	PFN_vkCmdCopyBuffer vkCmdCopyBuffer;
	PFN_vkCmdCopyImage vkCmdCopyImage;
	PFN_vkCmdBlitImage vkCmdBlitImage;
	PFN_vkCmdCopyBufferToImage vkCmdCopyBufferToImage;
	PFN_vkCmdCopyImageToBuffer vkCmdCopyImageToBuffer;
	PFN_vkCmdUpdateBuffer vkCmdUpdateBuffer;
	PFN_vkCmdFillBuffer vkCmdFillBuffer;
	PFN_vkCmdClearColorImage vkCmdClearColorImage;
	PFN_vkCmdClearDepthStencilImage vkCmdClearDepthStencilImage;
	PFN_vkCmdClearAttachments vkCmdClearAttachments;
	PFN_vkCmdResolveImage vkCmdResolveImage;
	PFN_vkCmdSetEvent vkCmdSetEvent;
	PFN_vkCmdResetEvent vkCmdResetEvent;
	PFN_vkCmdWaitEvents vkCmdWaitEvents;
	PFN_vkCmdPipelineBarrier vkCmdPipelineBarrier;
	PFN_vkCmdBeginQuery vkCmdBeginQuery;
	PFN_vkCmdEndQuery vkCmdEndQuery;
	PFN_vkCmdResetQueryPool vkCmdResetQueryPool;
	PFN_vkCmdWriteTimestamp vkCmdWriteTimestamp;
	PFN_vkCmdCopyQueryPoolResults vkCmdCopyQueryPoolResults;
	PFN_vkCmdPushConstants vkCmdPushConstants;
	PFN_vkCmdBeginRenderPass vkCmdBeginRenderPass;
	PFN_vkCmdNextSubpass vkCmdNextSubpass;
	PFN_vkCmdEndRenderPass vkCmdEndRenderPass;
	PFN_vkCmdExecuteCommands vkCmdExecuteCommands;

	// Vulkan 1.1
	PFN_vkEnumerateInstanceVersion vkEnumerateInstanceVersion;
	PFN_vkBindBufferMemory2 vkBindBufferMemory2;
	PFN_vkBindImageMemory2 vkBindImageMemory2;
	PFN_vkGetDeviceGroupPeerMemoryFeatures vkGetDeviceGroupPeerMemoryFeatures;
	PFN_vkCmdSetDeviceMask vkCmdSetDeviceMask;
	PFN_vkCmdDispatchBase vkCmdDispatchBase;
	PFN_vkEnumeratePhysicalDeviceGroups vkEnumeratePhysicalDeviceGroups;
	PFN_vkGetImageMemoryRequirements2 vkGetImageMemoryRequirements2;
	PFN_vkGetBufferMemoryRequirements2 vkGetBufferMemoryRequirements2;
	PFN_vkGetImageSparseMemoryRequirements2 vkGetImageSparseMemoryRequirements2;
	PFN_vkGetPhysicalDeviceFeatures2 vkGetPhysicalDeviceFeatures2;
	PFN_vkGetPhysicalDeviceProperties2 vkGetPhysicalDeviceProperties2;
	PFN_vkGetPhysicalDeviceFormatProperties2 vkGetPhysicalDeviceFormatProperties2;
	PFN_vkGetPhysicalDeviceImageFormatProperties2 vkGetPhysicalDeviceImageFormatProperties2;
	PFN_vkGetPhysicalDeviceQueueFamilyProperties2 vkGetPhysicalDeviceQueueFamilyProperties2;
	PFN_vkGetPhysicalDeviceMemoryProperties2 vkGetPhysicalDeviceMemoryProperties2;
	PFN_vkGetPhysicalDeviceSparseImageFormatProperties2 vkGetPhysicalDeviceSparseImageFormatProperties2;
	PFN_vkTrimCommandPool vkTrimCommandPool;
	PFN_vkGetDeviceQueue2 vkGetDeviceQueue2;
	PFN_vkCreateSamplerYcbcrConversion vkCreateSamplerYcbcrConversion;
	PFN_vkDestroySamplerYcbcrConversion vkDestroySamplerYcbcrConversion;
	PFN_vkCreateDescriptorUpdateTemplate vkCreateDescriptorUpdateTemplate;
	PFN_vkDestroyDescriptorUpdateTemplate vkDestroyDescriptorUpdateTemplate;
	PFN_vkUpdateDescriptorSetWithTemplate vkUpdateDescriptorSetWithTemplate;
	PFN_vkGetPhysicalDeviceExternalBufferProperties vkGetPhysicalDeviceExternalBufferProperties;
	PFN_vkGetPhysicalDeviceExternalFenceProperties vkGetPhysicalDeviceExternalFenceProperties;
	PFN_vkGetPhysicalDeviceExternalSemaphoreProperties vkGetPhysicalDeviceExternalSemaphoreProperties;
	PFN_vkGetDescriptorSetLayoutSupport vkGetDescriptorSetLayoutSupport;

	// Vulkan 1.2
	PFN_vkCmdDrawIndirectCount vkCmdDrawIndirectCount;
	PFN_vkCmdDrawIndexedIndirectCount vkCmdDrawIndexedIndirectCount;
	PFN_vkCreateRenderPass2 vkCreateRenderPass2;
	PFN_vkCmdBeginRenderPass2 vkCmdBeginRenderPass2;
	PFN_vkCmdNextSubpass2 vkCmdNextSubpass2;
	PFN_vkCmdEndRenderPass2 vkCmdEndRenderPass2;
	PFN_vkResetQueryPool vkResetQueryPool;
	PFN_vkGetSemaphoreCounterValue vkGetSemaphoreCounterValue;
	PFN_vkWaitSemaphores vkWaitSemaphores;
	PFN_vkSignalSemaphore vkSignalSemaphore;
	PFN_vkGetBufferDeviceAddress vkGetBufferDeviceAddress;
	PFN_vkGetBufferOpaqueCaptureAddress vkGetBufferOpaqueCaptureAddress;
	PFN_vkGetDeviceMemoryOpaqueCaptureAddress vkGetDeviceMemoryOpaqueCaptureAddress;

	// Vulkan 1.3
	PFN_vkGetPhysicalDeviceToolProperties vkGetPhysicalDeviceToolProperties;
	PFN_vkCreatePrivateDataSlot vkCreatePrivateDataSlot;
	PFN_vkDestroyPrivateDataSlot vkDestroyPrivateDataSlot;
	PFN_vkSetPrivateData vkSetPrivateData;
	PFN_vkGetPrivateData vkGetPrivateData;
	PFN_vkCmdSetEvent2 vkCmdSetEvent2;
	PFN_vkCmdResetEvent2 vkCmdResetEvent2;
	PFN_vkCmdWaitEvents2 vkCmdWaitEvents2;
	PFN_vkCmdPipelineBarrier2 vkCmdPipelineBarrier2;
	PFN_vkCmdWriteTimestamp2 vkCmdWriteTimestamp2;
	PFN_vkQueueSubmit2 vkQueueSubmit2;
	PFN_vkCmdCopyBuffer2 vkCmdCopyBuffer2;
	PFN_vkCmdCopyImage2 vkCmdCopyImage2;
	PFN_vkCmdCopyBufferToImage2 vkCmdCopyBufferToImage2;
	PFN_vkCmdCopyImageToBuffer2 vkCmdCopyImageToBuffer2;
	PFN_vkCmdBlitImage2 vkCmdBlitImage2;
	PFN_vkCmdResolveImage2 vkCmdResolveImage2;
	PFN_vkCmdBeginRendering vkCmdBeginRendering;
	PFN_vkCmdEndRendering vkCmdEndRendering;
	PFN_vkCmdSetCullMode vkCmdSetCullMode;
	PFN_vkCmdSetFrontFace vkCmdSetFrontFace;
	PFN_vkCmdSetPrimitiveTopology vkCmdSetPrimitiveTopology;
	PFN_vkCmdSetViewportWithCount vkCmdSetViewportWithCount;
	PFN_vkCmdSetScissorWithCount vkCmdSetScissorWithCount;
	PFN_vkCmdBindVertexBuffers2 vkCmdBindVertexBuffers2;
	PFN_vkCmdSetDepthTestEnable vkCmdSetDepthTestEnable;
	PFN_vkCmdSetDepthWriteEnable vkCmdSetDepthWriteEnable;
	PFN_vkCmdSetDepthCompareOp vkCmdSetDepthCompareOp;
	PFN_vkCmdSetDepthBoundsTestEnable vkCmdSetDepthBoundsTestEnable;
	PFN_vkCmdSetStencilTestEnable vkCmdSetStencilTestEnable;
	PFN_vkCmdSetStencilOp vkCmdSetStencilOp;
	PFN_vkCmdSetRasterizerDiscardEnable vkCmdSetRasterizerDiscardEnable;
	PFN_vkCmdSetDepthBiasEnable vkCmdSetDepthBiasEnable;
	PFN_vkCmdSetPrimitiveRestartEnable vkCmdSetPrimitiveRestartEnable;
	PFN_vkGetDeviceBufferMemoryRequirements vkGetDeviceBufferMemoryRequirements;
	PFN_vkGetDeviceImageMemoryRequirements vkGetDeviceImageMemoryRequirements;
	PFN_vkGetDeviceImageSparseMemoryRequirements vkGetDeviceImageSparseMemoryRequirements;

	// VK_KHR_surface
	PFN_vkDestroySurfaceKHR vkDestroySurfaceKHR;
	PFN_vkGetPhysicalDeviceSurfaceSupportKHR vkGetPhysicalDeviceSurfaceSupportKHR;
	PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR vkGetPhysicalDeviceSurfaceCapabilitiesKHR;
	PFN_vkGetPhysicalDeviceSurfaceFormatsKHR vkGetPhysicalDeviceSurfaceFormatsKHR;
	PFN_vkGetPhysicalDeviceSurfacePresentModesKHR vkGetPhysicalDeviceSurfacePresentModesKHR;

	// VK_KHR_swapchain
	PFN_vkCreateSwapchainKHR vkCreateSwapchainKHR;
	PFN_vkDestroySwapchainKHR vkDestroySwapchainKHR;
	PFN_vkGetSwapchainImagesKHR vkGetSwapchainImagesKHR;
	PFN_vkAcquireNextImageKHR vkAcquireNextImageKHR;
	PFN_vkQueuePresentKHR vkQueuePresentKHR;

	// VK_KHR_display
	PFN_vkGetPhysicalDeviceDisplayPropertiesKHR vkGetPhysicalDeviceDisplayPropertiesKHR;
	PFN_vkGetPhysicalDeviceDisplayPlanePropertiesKHR vkGetPhysicalDeviceDisplayPlanePropertiesKHR;
	PFN_vkGetDisplayPlaneSupportedDisplaysKHR vkGetDisplayPlaneSupportedDisplaysKHR;
	PFN_vkGetDisplayModePropertiesKHR vkGetDisplayModePropertiesKHR;
	PFN_vkCreateDisplayModeKHR vkCreateDisplayModeKHR;
	PFN_vkGetDisplayPlaneCapabilitiesKHR vkGetDisplayPlaneCapabilitiesKHR;
	PFN_vkCreateDisplayPlaneSurfaceKHR vkCreateDisplayPlaneSurfaceKHR;

	// VK_KHR_display_swapchain
	PFN_vkCreateSharedSwapchainsKHR vkCreateSharedSwapchainsKHR;

	// VK_KHR_xlib_surface
	#ifdef HAM_VK_XLIB
		PFN_vkCreateXlibSurfaceKHR vkCreateXlibSurfaceKHR;
		PFN_vkGetPhysicalDeviceXlibPresentationSupportKHR vkGetPhysicalDeviceXlibPresentationSupportKHR;
	#endif // HAM_VK_XLIB

	// VK_KHR_xcb_surface
	#ifdef HAM_VK_XCB
		PFN_vkCreateXcbSurfaceKHR vkCreateXcbSurfaceKHR;
		PFN_vkGetPhysicalDeviceXcbPresentationSupportKHR vkGetPhysicalDeviceXcbPresentationSupportKHR;
	#endif // HAM_VK_XCB

	// VK_KHR_wayland_surface
	#ifdef HAM_VK_WAYLAND
		PFN_vkCreateWaylandSurfaceKHR vkCreateWaylandSurfaceKHR;
		PFN_vkGetPhysicalDeviceWaylandPresentationSupportKHR vkGetPhysicalDeviceWaylandPresentationSupportKHR;
	#endif // HAM_VK_WAYLAND

	// VK_KHR_android_surface
	#ifdef HAM_VK_ANDROID
		PFN_vkCreateAndroidSurfaceKHR vkCreateAndroidSurfaceKHR;
	#endif // HAM_VK_ANDROID

	// VK_KHR_win32_surface
	#ifdef HAM_VK_WIN32
		PFN_vkCreateWin32SurfaceKHR vkCreateWin32SurfaceKHR;
		PFN_vkGetPhysicalDeviceWin32PresentationSupportKHR vkGetPhysicalDeviceWin32PresentationSupportKHR;
	#endif // HAM_VK_WIN32

	// VK_KHR_video_queue
	#ifdef HAM_VK_VIDEO
		PFN_vkGetPhysicalDeviceVideoCapabilitiesKHR vkGetPhysicalDeviceVideoCapabilitiesKHR;
		PFN_vkGetPhysicalDeviceVideoFormatPropertiesKHR vkGetPhysicalDeviceVideoFormatPropertiesKHR;
		PFN_vkCreateVideoSessionKHR vkCreateVideoSessionKHR;
		PFN_vkDestroyVideoSessionKHR vkDestroyVideoSessionKHR;
		PFN_vkGetVideoSessionMemoryRequirementsKHR vkGetVideoSessionMemoryRequirementsKHR;
		PFN_vkBindVideoSessionMemoryKHR vkBindVideoSessionMemoryKHR;
		PFN_vkCreateVideoSessionParametersKHR vkCreateVideoSessionParametersKHR;
		PFN_vkUpdateVideoSessionParametersKHR vkUpdateVideoSessionParametersKHR;
		PFN_vkDestroyVideoSessionParametersKHR vkDestroyVideoSessionParametersKHR;
		PFN_vkCmdBeginVideoCodingKHR vkCmdBeginVideoCodingKHR;
		PFN_vkCmdEndVideoCodingKHR vkCmdEndVideoCodingKHR;
		PFN_vkCmdControlVideoCodingKHR vkCmdControlVideoCodingKHR;
	#endif // HAM_VK_VIDEO

	// VK_KHR_video_decode_queue
	#ifdef HAM_VK_VIDEO
		PFN_vkCmdDecodeVideoKHR vkCmdDecodeVideoKHR;
	#endif // HAM_VK_VIDEO

	// VK_KHR_dynamic_rendering
	PFN_vkCmdBeginRenderingKHR vkCmdBeginRenderingKHR;
	PFN_vkCmdEndRenderingKHR vkCmdEndRenderingKHR;

	// VK_KHR_get_physical_device_properties2
	PFN_vkGetPhysicalDeviceFeatures2KHR vkGetPhysicalDeviceFeatures2KHR;
	PFN_vkGetPhysicalDeviceProperties2KHR vkGetPhysicalDeviceProperties2KHR;
	PFN_vkGetPhysicalDeviceFormatProperties2KHR vkGetPhysicalDeviceFormatProperties2KHR;
	PFN_vkGetPhysicalDeviceImageFormatProperties2KHR vkGetPhysicalDeviceImageFormatProperties2KHR;
	PFN_vkGetPhysicalDeviceQueueFamilyProperties2KHR vkGetPhysicalDeviceQueueFamilyProperties2KHR;
	PFN_vkGetPhysicalDeviceMemoryProperties2KHR vkGetPhysicalDeviceMemoryProperties2KHR;
	PFN_vkGetPhysicalDeviceSparseImageFormatProperties2KHR vkGetPhysicalDeviceSparseImageFormatProperties2KHR;

	// VK_KHR_device_group
	PFN_vkGetDeviceGroupPeerMemoryFeaturesKHR vkGetDeviceGroupPeerMemoryFeaturesKHR;
	PFN_vkCmdSetDeviceMaskKHR vkCmdSetDeviceMaskKHR;
	PFN_vkCmdDispatchBaseKHR vkCmdDispatchBaseKHR;
	PFN_vkGetDeviceGroupPresentCapabilitiesKHR vkGetDeviceGroupPresentCapabilitiesKHR;
	PFN_vkGetDeviceGroupSurfacePresentModesKHR vkGetDeviceGroupSurfacePresentModesKHR;
	PFN_vkGetPhysicalDevicePresentRectanglesKHR vkGetPhysicalDevicePresentRectanglesKHR;
	PFN_vkAcquireNextImage2KHR vkAcquireNextImage2KHR;

	// VK_KHR_maintenance1
	PFN_vkTrimCommandPoolKHR vkTrimCommandPoolKHR;

	// VK_KHR_device_group_creation
	PFN_vkEnumeratePhysicalDeviceGroupsKHR vkEnumeratePhysicalDeviceGroupsKHR;

	// VK_KHR_external_memory_capabilities
	PFN_vkGetPhysicalDeviceExternalBufferPropertiesKHR vkGetPhysicalDeviceExternalBufferPropertiesKHR;

	// VK_KHR_external_memory_win32
	#ifdef HAM_VK_WIN32
		PFN_vkGetMemoryWin32HandleKHR vkGetMemoryWin32HandleKHR;
		PFN_vkGetMemoryWin32HandlePropertiesKHR vkGetMemoryWin32HandlePropertiesKHR;
	#endif // HAM_VK_WIN32

	// VK_KHR_external_memory_fd
	PFN_vkGetMemoryFdKHR vkGetMemoryFdKHR;
	PFN_vkGetMemoryFdPropertiesKHR vkGetMemoryFdPropertiesKHR;

	// VK_KHR_external_semaphore_capabilities
	PFN_vkGetPhysicalDeviceExternalSemaphorePropertiesKHR vkGetPhysicalDeviceExternalSemaphorePropertiesKHR;

	// VK_KHR_external_semaphore_win32
	#ifdef HAM_VK_WIN32
		PFN_vkImportSemaphoreWin32HandleKHR vkImportSemaphoreWin32HandleKHR;
		PFN_vkGetSemaphoreWin32HandleKHR vkGetSemaphoreWin32HandleKHR;
	#endif // HAM_VK_WIN32

	// VK_KHR_external_semaphore_fd
	PFN_vkImportSemaphoreFdKHR vkImportSemaphoreFdKHR;
	PFN_vkGetSemaphoreFdKHR vkGetSemaphoreFdKHR;

	// VK_KHR_push_descriptor
	PFN_vkCmdPushDescriptorSetKHR vkCmdPushDescriptorSetKHR;

	// VK_KHR_descriptor_update_template
	PFN_vkCreateDescriptorUpdateTemplateKHR vkCreateDescriptorUpdateTemplateKHR;
	PFN_vkDestroyDescriptorUpdateTemplateKHR vkDestroyDescriptorUpdateTemplateKHR;
	PFN_vkUpdateDescriptorSetWithTemplateKHR vkUpdateDescriptorSetWithTemplateKHR;
	PFN_vkCmdPushDescriptorSetWithTemplateKHR vkCmdPushDescriptorSetWithTemplateKHR;

	// VK_KHR_create_renderpass2
	PFN_vkCreateRenderPass2KHR vkCreateRenderPass2KHR;
	PFN_vkCmdBeginRenderPass2KHR vkCmdBeginRenderPass2KHR;
	PFN_vkCmdNextSubpass2KHR vkCmdNextSubpass2KHR;
	PFN_vkCmdEndRenderPass2KHR vkCmdEndRenderPass2KHR;

	// VK_KHR_shared_presentable_image
	PFN_vkGetSwapchainStatusKHR vkGetSwapchainStatusKHR;

	// VK_KHR_external_fence_capabilities
	PFN_vkGetPhysicalDeviceExternalFencePropertiesKHR vkGetPhysicalDeviceExternalFencePropertiesKHR;

	// VK_KHR_external_fence_win32
	#ifdef HAM_VK_WIN32
		PFN_vkImportFenceWin32HandleKHR vkImportFenceWin32HandleKHR;
		PFN_vkGetFenceWin32HandleKHR vkGetFenceWin32HandleKHR;
	#endif // HAM_VK_WIN32

	// VK_KHR_external_fence_fd
	PFN_vkImportFenceFdKHR vkImportFenceFdKHR;
	PFN_vkGetFenceFdKHR vkGetFenceFdKHR;

	// VK_KHR_performance_query
	PFN_vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR;
	PFN_vkGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR vkGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR;
	PFN_vkAcquireProfilingLockKHR vkAcquireProfilingLockKHR;
	PFN_vkReleaseProfilingLockKHR vkReleaseProfilingLockKHR;

	// VK_KHR_get_surface_capabilities2
	PFN_vkGetPhysicalDeviceSurfaceCapabilities2KHR vkGetPhysicalDeviceSurfaceCapabilities2KHR;
	PFN_vkGetPhysicalDeviceSurfaceFormats2KHR vkGetPhysicalDeviceSurfaceFormats2KHR;

	// VK_KHR_get_display_properties2
	PFN_vkGetPhysicalDeviceDisplayProperties2KHR vkGetPhysicalDeviceDisplayProperties2KHR;
	PFN_vkGetPhysicalDeviceDisplayPlaneProperties2KHR vkGetPhysicalDeviceDisplayPlaneProperties2KHR;
	PFN_vkGetDisplayModeProperties2KHR vkGetDisplayModeProperties2KHR;
	PFN_vkGetDisplayPlaneCapabilities2KHR vkGetDisplayPlaneCapabilities2KHR;

	// VK_KHR_get_memory_requirements2
	PFN_vkGetImageMemoryRequirements2KHR vkGetImageMemoryRequirements2KHR;
	PFN_vkGetBufferMemoryRequirements2KHR vkGetBufferMemoryRequirements2KHR;
	PFN_vkGetImageSparseMemoryRequirements2KHR vkGetImageSparseMemoryRequirements2KHR;

	// VK_KHR_acceleration_structure
	PFN_vkCreateAccelerationStructureKHR vkCreateAccelerationStructureKHR;
	PFN_vkDestroyAccelerationStructureKHR vkDestroyAccelerationStructureKHR;
	PFN_vkCmdBuildAccelerationStructuresKHR vkCmdBuildAccelerationStructuresKHR;
	PFN_vkCmdBuildAccelerationStructuresIndirectKHR vkCmdBuildAccelerationStructuresIndirectKHR;
	PFN_vkBuildAccelerationStructuresKHR vkBuildAccelerationStructuresKHR;
	PFN_vkCopyAccelerationStructureKHR vkCopyAccelerationStructureKHR;
	PFN_vkCopyAccelerationStructureToMemoryKHR vkCopyAccelerationStructureToMemoryKHR;
	PFN_vkCopyMemoryToAccelerationStructureKHR vkCopyMemoryToAccelerationStructureKHR;
	PFN_vkWriteAccelerationStructuresPropertiesKHR vkWriteAccelerationStructuresPropertiesKHR;
	PFN_vkCmdCopyAccelerationStructureKHR vkCmdCopyAccelerationStructureKHR;
	PFN_vkCmdCopyAccelerationStructureToMemoryKHR vkCmdCopyAccelerationStructureToMemoryKHR;
	PFN_vkCmdCopyMemoryToAccelerationStructureKHR vkCmdCopyMemoryToAccelerationStructureKHR;
	PFN_vkGetAccelerationStructureDeviceAddressKHR vkGetAccelerationStructureDeviceAddressKHR;
	PFN_vkCmdWriteAccelerationStructuresPropertiesKHR vkCmdWriteAccelerationStructuresPropertiesKHR;
	PFN_vkGetDeviceAccelerationStructureCompatibilityKHR vkGetDeviceAccelerationStructureCompatibilityKHR;
	PFN_vkGetAccelerationStructureBuildSizesKHR vkGetAccelerationStructureBuildSizesKHR;

	// VK_KHR_ray_tracing_pipeline
	PFN_vkCmdTraceRaysKHR vkCmdTraceRaysKHR;
	PFN_vkCreateRayTracingPipelinesKHR vkCreateRayTracingPipelinesKHR;
	PFN_vkGetRayTracingShaderGroupHandlesKHR vkGetRayTracingShaderGroupHandlesKHR;
	PFN_vkGetRayTracingCaptureReplayShaderGroupHandlesKHR vkGetRayTracingCaptureReplayShaderGroupHandlesKHR;
	PFN_vkCmdTraceRaysIndirectKHR vkCmdTraceRaysIndirectKHR;
	PFN_vkGetRayTracingShaderGroupStackSizeKHR vkGetRayTracingShaderGroupStackSizeKHR;
	PFN_vkCmdSetRayTracingPipelineStackSizeKHR vkCmdSetRayTracingPipelineStackSizeKHR;

	// VK_KHR_sampler_ycbcr_conversion
	PFN_vkCreateSamplerYcbcrConversionKHR vkCreateSamplerYcbcrConversionKHR;
	PFN_vkDestroySamplerYcbcrConversionKHR vkDestroySamplerYcbcrConversionKHR;

	// VK_KHR_bind_memory2
	PFN_vkBindBufferMemory2KHR vkBindBufferMemory2KHR;
	PFN_vkBindImageMemory2KHR vkBindImageMemory2KHR;

	// VK_KHR_maintenance3
	PFN_vkGetDescriptorSetLayoutSupportKHR vkGetDescriptorSetLayoutSupportKHR;

	// VK_KHR_draw_indirect_count
	PFN_vkCmdDrawIndirectCountKHR vkCmdDrawIndirectCountKHR;
	PFN_vkCmdDrawIndexedIndirectCountKHR vkCmdDrawIndexedIndirectCountKHR;

	// VK_KHR_timeline_semaphore
	PFN_vkGetSemaphoreCounterValueKHR vkGetSemaphoreCounterValueKHR;
	PFN_vkWaitSemaphoresKHR vkWaitSemaphoresKHR;
	PFN_vkSignalSemaphoreKHR vkSignalSemaphoreKHR;

	// VK_KHR_fragment_shading_rate
	PFN_vkGetPhysicalDeviceFragmentShadingRatesKHR vkGetPhysicalDeviceFragmentShadingRatesKHR;
	PFN_vkCmdSetFragmentShadingRateKHR vkCmdSetFragmentShadingRateKHR;

	// VK_KHR_present_wait
	PFN_vkWaitForPresentKHR vkWaitForPresentKHR;

	// VK_KHR_buffer_device_address
	PFN_vkGetBufferDeviceAddressKHR vkGetBufferDeviceAddressKHR;
	PFN_vkGetBufferOpaqueCaptureAddressKHR vkGetBufferOpaqueCaptureAddressKHR;
	PFN_vkGetDeviceMemoryOpaqueCaptureAddressKHR vkGetDeviceMemoryOpaqueCaptureAddressKHR;

	// VK_KHR_deferred_host_operations
	PFN_vkCreateDeferredOperationKHR vkCreateDeferredOperationKHR;
	PFN_vkDestroyDeferredOperationKHR vkDestroyDeferredOperationKHR;
	PFN_vkGetDeferredOperationMaxConcurrencyKHR vkGetDeferredOperationMaxConcurrencyKHR;
	PFN_vkGetDeferredOperationResultKHR vkGetDeferredOperationResultKHR;
	PFN_vkDeferredOperationJoinKHR vkDeferredOperationJoinKHR;

	// VK_KHR_pipeline_executable_properties
	PFN_vkGetPipelineExecutablePropertiesKHR vkGetPipelineExecutablePropertiesKHR;
	PFN_vkGetPipelineExecutableStatisticsKHR vkGetPipelineExecutableStatisticsKHR;
	PFN_vkGetPipelineExecutableInternalRepresentationsKHR vkGetPipelineExecutableInternalRepresentationsKHR;

	// VK_KHR_video_encode_queue
	#ifdef HAM_VK_VIDEO
		PFN_vkCmdEncodeVideoKHR vkCmdEncodeVideoKHR;
	#endif // HAM_VK_VIDEO

	// VK_KHR_synchronization2
	PFN_vkCmdSetEvent2KHR vkCmdSetEvent2KHR;
	PFN_vkCmdResetEvent2KHR vkCmdResetEvent2KHR;
	PFN_vkCmdWaitEvents2KHR vkCmdWaitEvents2KHR;
	PFN_vkCmdPipelineBarrier2KHR vkCmdPipelineBarrier2KHR;
	PFN_vkCmdWriteTimestamp2KHR vkCmdWriteTimestamp2KHR;
	PFN_vkQueueSubmit2KHR vkQueueSubmit2KHR;
	PFN_vkCmdWriteBufferMarker2AMD vkCmdWriteBufferMarker2AMD;
	PFN_vkGetQueueCheckpointData2NV vkGetQueueCheckpointData2NV;

	// VK_KHR_copy_commands2
	PFN_vkCmdCopyBuffer2KHR vkCmdCopyBuffer2KHR;
	PFN_vkCmdCopyImage2KHR vkCmdCopyImage2KHR;
	PFN_vkCmdCopyBufferToImage2KHR vkCmdCopyBufferToImage2KHR;
	PFN_vkCmdCopyImageToBuffer2KHR vkCmdCopyImageToBuffer2KHR;
	PFN_vkCmdBlitImage2KHR vkCmdBlitImage2KHR;
	PFN_vkCmdResolveImage2KHR vkCmdResolveImage2KHR;

	// VK_KHR_ray_tracing_maintenance1
	PFN_vkCmdTraceRaysIndirect2KHR vkCmdTraceRaysIndirect2KHR;

	// VK_KHR_maintenance4
	PFN_vkGetDeviceBufferMemoryRequirementsKHR vkGetDeviceBufferMemoryRequirementsKHR;
	PFN_vkGetDeviceImageMemoryRequirementsKHR vkGetDeviceImageMemoryRequirementsKHR;
	PFN_vkGetDeviceImageSparseMemoryRequirementsKHR vkGetDeviceImageSparseMemoryRequirementsKHR;
} ham_vk_fns;

HAM_C_API_END

#endif // !HAM_VK_FNS_H
