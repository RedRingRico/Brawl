#ifndef __BRAWL_DATATYPES_HPP__
#error This file should not be included directly (ErrorCodes.hpp).  Include DataTypes.hpp, instead.
#endif // __BRAWL_DATATYPES_HPP__

#ifndef __BRAWL_ERRORCODES_HPP__
#define __BRAWL_ERRORCODES_HPP__

namespace Brawl
{
	enum class ErrorCode : SInt32
	{
		// Windowing system (XCB)
		XCBOpenDisplayFailed = -32000,
		PrimaryOutputFailure,
		PrimaryOutputInfoFailure,

		// Renderer
		InitialiseFailed = -16000,
		LoadVulkanLibraryFailed,
		LoadVulkanEntryPointsFailed,
		LoadVulkanGlobalLevelEntryPointsFailed,
		CreateVulkanInstanceFailed,
		RequiredVulkanExtensionUnavailable,
		LoadVulkanInstanceLevelEntryPointsFailed,
		CreateVulkanPresentationSurfaceFailed,
		PhysicalVulkanDeviceSelectionFailed,
		CreateVulkanDeviceFailed,
		PhysicalVulkanDeviceEnumerationFailed,
		LoadVulkanDeviceLevelEntryPointsFailed,
		GetVulkanDeviceQueuesFailed,
		CreateVulkanSemaphoresFailed,
		NoVulkanDeviceSurfaceCapabilities,
		VulkanDeviceSurfaceFormatCountFailed,
		VulkanDeviceSurfaceFormatEnumerationFailed,
		VulkanDevicePresentationModesCountFailed,
		VulkanDevicePresentationModesEnumerationFailed,
		IncorrectVulkanImageUsageFlags,
		IncorrectVulkanPresentMode,
		CreateVulkanSwapchainFailed,
		CreateVulkanImageViewFailed,
		CreateVulkanRenderPassFailed,
		CreateVulkanGraphicsPipelineFailed,
		CreateVulkanShaderModuleFailed,
		VulkanCreatePipelineLayoutFailed,
		VulkanCreateGraphicsPipelineFailed,
		CreateVulkanFramebuffersFailed,
		VulkanCreateFramebufferFailed,
		CreateVulkanCommandPoolFailed,
		VulkanCreateCommandPoolFailed,
		CreateVulkanCommandBuffersFailed,
		VulkanAllocateCommandBuffersFailed,
		VulkanBeginCommandBufferFailed,
		VulkanEndCommandBufferFailed,
		CreateVulkanSemaporesFailed,
		VulkanCreateSemaphoreFailed,
		VulkanQueueSubmitFailed,
		MissingVulkanValidationLayers,
		VulkanCreateBufferFailed,
		VulkanAllocateMemoryFailed,
		CreateVulkanVertexBufferFailed,
		CreateBVulkanIndexBufferFailed,

		// System
		CloseLibraryFailed = -1000,
		// File I/O
		FileSystemAlreadyInitialised,
		InvalidFileAccessFlags,
		InvalidFileHandle,
		FileNotOpen,
		FileRewindFailed,
		ReadBytesFailed,
		WriteBytesFailed,
		MalformedFileURI,
		UnknownFileURI,
		OpenFileFailed,
		FileURIHandlerUndefined,
		NativeFileContainerOpenFailed,

		// Game
		WindowCreationFailed = -100,

		// General
		FatalError = -1,
		Okay = 0,
		Error = 1,
	};
}

#endif // __BRAWL_ERRORCODES_HPP__

