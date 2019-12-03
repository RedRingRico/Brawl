#include <Renderer.hpp>
#include <System.hpp>
#include <VulkanFunctions.hpp>
#include <File.hpp>
#include <FileSystem.hpp>

#if defined( VK_USE_PLATFORM_XCB_KHR )
#include <GameWindowXCB.hpp>
#endif // VK_USE_PLATFORM_XCB_KHR

#include <iostream>
#include <cstring>
#include <limits>

namespace Brawl
{
	VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
		    auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
			    if (func != nullptr) {
					        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
							    } else {
									        return VK_ERROR_EXTENSION_NOT_PRESENT;
											    }
	}

	void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
		    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
			    if (func != nullptr) {
					        func(instance, debugMessenger, pAllocator);
							    }
	}

	static const int32_t kMaxFramesInFlight = 2;

	Renderer::Renderer( ) :
		//m_pGameWindowData( Null ),
		m_pGameWindow( Null ),
		m_VulkanLibrary( InvalidLibraryHandle ),
		m_VulkanInstance( VK_NULL_HANDLE ),
		m_DebugMessenger( VK_NULL_HANDLE ),
		m_VulkanPhysicalDevice( VK_NULL_HANDLE ),
		m_VulkanDevice( VK_NULL_HANDLE ),
		m_GraphicsQueue( VK_NULL_HANDLE ),
		m_GraphicsQueueFamilyIndex( 0UL ),
		m_PresentQueue( VK_NULL_HANDLE ),
		m_PresentQueueFamilyIndex( 0UL ),
		m_PresentationSurface( VK_NULL_HANDLE ),
		m_Swapchain( VK_NULL_HANDLE ),
		m_SwapchainExtent( { 0, 0 } ),
		m_PipelineLayout( VK_NULL_HANDLE ),
		m_GraphicsPipeline( VK_NULL_HANDLE ),
		m_RenderPass( VK_NULL_HANDLE ),
		m_CommandPool( VK_NULL_HANDLE ),
		m_CurrentFrame( 0 ),
		m_CanRender( False ),
		m_Resize( False )
	{
	}

	Renderer::~Renderer( )
	{
		this->Terminate( );
	}

	ErrorCode Renderer::Initialise( GameWindow *p_pGameWindow )
	{
		if( this->LoadVulkanLibrary( ) != ErrorCode::Okay )
		{
			return ErrorCode::LoadVulkanLibraryFailed;
		}

		if( this->LoadExportedEntryPoints( ) != ErrorCode::Okay )
		{
			return ErrorCode::LoadVulkanEntryPointsFailed;
		}

		if( this->LoadGlobalLevelEntryPoints( ) != ErrorCode::Okay )
		{
			return ErrorCode::LoadVulkanGlobalLevelEntryPointsFailed;
		}

		if( this->CreateVulkanInstance( ) != ErrorCode::Okay )
		{
			return ErrorCode::CreateVulkanInstanceFailed;
		}

		if( this->LoadInstanceLevelEntryPoints( ) != ErrorCode::Okay )
		{
			return ErrorCode::LoadVulkanInstanceLevelEntryPointsFailed;
		}

		//m_pGameWindowData = p_pGameWindow->GetGameWindowData( );
		m_pGameWindow = p_pGameWindow;

		if( this->CreatePresentationSurface( ) != ErrorCode::Okay )
		{
			return ErrorCode::CreateVulkanPresentationSurfaceFailed;
		}

		if( this->CreateVulkanDevice( ) != ErrorCode::Okay )
		{
			return ErrorCode::CreateVulkanDeviceFailed;
		}

		if( this->LoadDeviceLevelEntryPoints( ) != ErrorCode::Okay )
		{
			return ErrorCode::LoadVulkanDeviceLevelEntryPointsFailed;
		}

		if( this->GetDeviceQueues( ) != ErrorCode::Okay )
		{
			return ErrorCode::GetVulkanDeviceQueuesFailed;
		}

		if( this->CreateSwapchain( ) != ErrorCode::Okay )
		{
			return ErrorCode::CreateVulkanSwapchainFailed;
		}

		if( this->CreateSwapchainImageViews( ) != ErrorCode::Okay )
		{
			return ErrorCode::CreateVulkanImageViewFailed;
		}

		if( this->CreateRenderPass( ) != ErrorCode::Okay )
		{
			return ErrorCode::CreateVulkanRenderPassFailed;
		}

		if( this->CreateGraphicsPipeline( ) != ErrorCode::Okay )
		{
			return ErrorCode::CreateVulkanGraphicsPipelineFailed;
		}

		if( this->CreateFramebuffers( ) != ErrorCode::Okay )
		{
			return ErrorCode::CreateVulkanFramebuffersFailed;
		}

		if( this->CreateCommandPool( ) != ErrorCode::Okay )
		{
			return ErrorCode::CreateVulkanCommandPoolFailed;
		}

		if( this->CreateCommandBuffers( ) != ErrorCode::Okay )
		{
			return ErrorCode::CreateVulkanCommandBuffersFailed;
		}

		if( this->CreateSemaphores( ) != ErrorCode::Okay )
		{
			return ErrorCode::CreateVulkanSemaporesFailed;
		}

		p_pGameWindow->SetRenderer( this );

		return ErrorCode::Okay;
	}

	void Renderer::Terminate( )
	{
		this->CleanupSwapchain( );

		for( size_t Frame = 0; Frame < kMaxFramesInFlight; ++Frame )
		{
			vkDestroySemaphore( m_VulkanDevice,
				m_RenderingFinishedSemaphores[ Frame ], Null );
			vkDestroySemaphore( m_VulkanDevice,
				m_ImageAvailableSemaphores[ Frame ], Null );
			vkDestroyFence( m_VulkanDevice, m_InFlightFences[ Frame ], Null );
		}

		if( m_CommandPool != VK_NULL_HANDLE )
		{
			vkDestroyCommandPool( m_VulkanDevice, m_CommandPool, Null );
			m_CommandPool = VK_NULL_HANDLE;
		}

		if( m_VulkanDevice != VK_NULL_HANDLE )
		{
			vkDestroyDevice( m_VulkanDevice, Null );
			m_VulkanDevice = VK_NULL_HANDLE;
		}

		if( m_PresentationSurface != VK_NULL_HANDLE )
		{
			vkDestroySurfaceKHR( m_VulkanInstance, m_PresentationSurface,
				Null );
			m_PresentationSurface = VK_NULL_HANDLE;
		}

		if( m_DebugMessenger != VK_NULL_HANDLE )
		{
			DestroyDebugUtilsMessengerEXT( m_VulkanInstance, m_DebugMessenger,
				Null );
			m_DebugMessenger = VK_NULL_HANDLE;
		}

		if( m_VulkanInstance != VK_NULL_HANDLE )
		{
			vkDestroyInstance( m_VulkanInstance, Null );
			m_VulkanInstance = VK_NULL_HANDLE;
		}

		if( m_VulkanLibrary != InvalidLibraryHandle )
		{
			CloseLibrary( m_VulkanLibrary );
			m_VulkanLibrary = InvalidLibraryHandle;
		}
	}

	ErrorCode Renderer::LoadVulkanLibrary( )
	{
		LibraryFlags OpenLibraryFlags = LibraryFlags::ImmediateBind;
		std::string LibraryName;
#if defined( BRAWL_PLATFORM_LINUX )
		LibraryName = "libvulkan.so.1";
#else
#error Unknown platform detected
#endif // BRAWL_PLATFORM_LINUX

		m_VulkanLibrary = OpenLibrary( LibraryName, OpenLibraryFlags );

		if( m_VulkanLibrary == InvalidLibraryHandle )
		{
			return ErrorCode::LoadVulkanLibraryFailed;
		}

		return ErrorCode::Okay;
	}

	ErrorCode Renderer::LoadExportedEntryPoints( )
	{
#define VK_EXPORTED_FUNCTION( p_Function )\
		if( !( p_Function = ( PFN_##p_Function )LoadProcAddress(\
			m_VulkanLibrary, #p_Function ) ) )\
		{\
			std::cout << "[Brawl::Renderer::LoadExportedEntryPoints] <ERROR> "\
				"Could not load function: \"" << #p_Function << "\"" <<\
				std::endl;\
			return ErrorCode::LoadVulkanEntryPointsFailed;\
		}\
		else\
		{\
			std::cout << "[Brawl::Renderer::LoadExportedEntryPoints] <INFO> "\
				"Loaded function \"" << #p_Function << "\"" << std::endl;\
		}

#include <VulkanFunctions.inl>
		return ErrorCode::Okay;
	}

	ErrorCode Renderer::LoadGlobalLevelEntryPoints( )
	{
#define VK_GLOBAL_LEVEL_FUNCTION( p_Function )\
		if( !( p_Function = ( PFN_##p_Function )vkGetInstanceProcAddr(\
			Null, #p_Function ) ) )\
		{\
			std::cout << "[Brawl::Renderer::LoadGlobalLevelEntryPoints] "\
				"<ERROR> Failed to load function: \"" << #p_Function <<\
				"\"" << std::endl;\
			return ErrorCode::LoadVulkanGlobalLevelEntryPointsFailed;\
		}\
		else\
		{\
			std::cout << "[Brawl::Renderer::LoadGlobalLevelEntryPoints] "\
				"<INFO> Loaded function: \"" << #p_Function << "\"" <<\
				std::endl;\
		}

#include <VulkanFunctions.inl>

		return ErrorCode::Okay;
	}

	static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
		        std::cout << "validation layer: " << pCallbackData->pMessage << std::endl;

				        return VK_FALSE;
						    }

	void PopulateDebugMessengerCreateInfo( VkDebugUtilsMessengerCreateInfoEXT &p_CreateInfo )
	{
		p_CreateInfo = { };
		p_CreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		p_CreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		p_CreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		p_CreateInfo.pfnUserCallback = DebugCallback;
	}

	ErrorCode Renderer::CreateVulkanInstance( )
	{
		std::vector< const char * > ValidationLayers =
		{
			"VK_LAYER_KHRONOS_validation",
			//"VK_LAYER_LUNARG_parameter_validation",
			"VK_LAYER_LUNARG_standard_validation"
		};

		if( CheckValidationLayerSupport( ValidationLayers ) == False )
		{
			std::cout << "[Brawl::Renderer::CreateVulkanDevice] <ERROR> "
				"One of the following validation layers was not found:" <<
				std::endl;

			for( auto &ValidationLayer : ValidationLayers )
			{
				std::cout << "\t" << ValidationLayer << std::endl;
			}

			return ErrorCode::MissingVulkanValidationLayers;
		}

		uint32_t ExtensionCount = 0;

		if( ( vkEnumerateInstanceExtensionProperties( Null, &ExtensionCount,
			Null ) != VK_SUCCESS ) || ( ExtensionCount == 0 ) )
		{
			std::cout << "[Brawl::Renderer::CreateVulkanInstance] <ERROR> "
				"Unable to get the extension count" << std::endl;

			return ErrorCode::CreateVulkanInstanceFailed;
		}

		std::vector< VkExtensionProperties > AvailableExtensions(
			ExtensionCount );

		if( vkEnumerateInstanceExtensionProperties( Null, &ExtensionCount,
			AvailableExtensions.data( ) ) != VK_SUCCESS )
		{
			std::cout << "[Brawl::Renderer::CreateVulkanInstance] <ERROR> "
				"Failed to enumerate the available extensions" << std::endl;

			return ErrorCode::CreateVulkanInstanceFailed;
		}

#if defined( BRAWL_BUILD_DEBUG )
		std::cout << "[Brawl::Renderer::CreateVulkanInstance] <INFO> "
			"Discovered " << AvailableExtensions.size( ) << " extensions" <<
			std::endl;

		for( const auto &Extension : AvailableExtensions )
		{
			std::cout << "\t" << Extension.extensionName << std::endl;
		}
#endif // BRAWL_BUILD_DEBUG

		std::vector< const char * > Extensions =
		{
#if defined( VK_USE_PLATFORM_XCB_KHR )
			VK_KHR_XCB_SURFACE_EXTENSION_NAME,
#endif // VK_USE_PLATFORM_XCB_KHR
			VK_KHR_SURFACE_EXTENSION_NAME
		};

		for( const auto &Extension : Extensions )
		{
			if( this->CheckExtensionAvailability( Extension,
				AvailableExtensions ) == False )
			{
				std::cout << "[Brawl::Renderer::CreateVulkanInstance] <ERROR> "
					"Extension \"" << Extension << "\" is not available" <<
					std::endl;

				return ErrorCode::RequiredVulkanExtensionUnavailable;
			}
		}

		VkApplicationInfo ApplicationInfo =
		{
			VK_STRUCTURE_TYPE_APPLICATION_INFO,
			Null,
			"Brawl",
			VK_MAKE_VERSION( 1, 0, 0 ),
			"Brawl",
			VK_MAKE_VERSION( 1, 0, 0 ),
			VK_MAKE_VERSION( 1, 0, 0 )
		};

		VkInstanceCreateInfo InstanceCreateInfo =
		{
			VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
			Null,
			0,
			&ApplicationInfo,
			0,
			Null,
			static_cast< uint32_t >( Extensions.size( ) ),
			Extensions.data( )
		};

		VkDebugUtilsMessengerCreateInfoEXT DebugCreateInfo;

		InstanceCreateInfo.enabledLayerCount =
			static_cast< uint32_t >( ValidationLayers.size( ) );
		InstanceCreateInfo.ppEnabledLayerNames = ValidationLayers.data( );
		PopulateDebugMessengerCreateInfo( DebugCreateInfo );
		InstanceCreateInfo.pNext = ( VkDebugUtilsMessengerCreateInfoEXT * )&DebugCreateInfo;

		if( vkCreateInstance( &InstanceCreateInfo, Null, &m_VulkanInstance ) !=
			VK_SUCCESS )
		{
			std::cout << "[Brawl::Renderer::CreateVulkanInstance] <ERROR> "
				"Failed to create the Vulkan instance" << std::endl;

			return ErrorCode::CreateVulkanInstanceFailed;
		}

		CreateDebugUtilsMessengerEXT( m_VulkanInstance, &DebugCreateInfo, Null, &m_DebugMessenger );

		return ErrorCode::Okay;
	}

	ErrorCode Renderer::LoadInstanceLevelEntryPoints( )
	{
#define VK_INSTANCE_LEVEL_FUNCTION( p_Function )\
		if( !( p_Function = ( PFN_##p_Function )vkGetInstanceProcAddr(\
			m_VulkanInstance, #p_Function ) ) )\
		{\
			std::cout << "[Brawl::Renderer::LoadInstanceLevelEntryPoints] "\
				"<ERROR> Failed to load function: \"" << #p_Function <<\
				"\"" << std::endl;\
			return ErrorCode::LoadVulkanInstanceLevelEntryPointsFailed;\
		}\
		else\
		{\
			std::cout << "[Brawl::Renderer::LoadInstanceLevelEntryPoints] "\
				"<INFO> Loaded function: \"" << #p_Function << "\"" <<\
				std::endl;\
		}

#include <VulkanFunctions.inl>

		return ErrorCode::Okay;
	}

	ErrorCode Renderer::CreatePresentationSurface( )
	{
#if defined( VK_USE_PLATFORM_XCB_KHR )
		GameWindowDataXCB *pGameWindowData =
			reinterpret_cast< GameWindowDataXCB * >( m_pGameWindow->GetGameWindowData( ) );

		VkXcbSurfaceCreateInfoKHR SurfaceCreateInfo =
		{
			VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR,
			Null,
			0,
			pGameWindowData->GetConnection( ),
			pGameWindowData->GetWindow( )
		};

		if( vkCreateXcbSurfaceKHR( m_VulkanInstance, &SurfaceCreateInfo, Null,
			&m_PresentationSurface ) == VK_SUCCESS )
		{
			return ErrorCode::Okay;
		}

#endif // VK_USE_PLATFORM_XCB_KHR
	
		std::cout << "[Brawl::Renderer::CreatePresentationSurface] <ERROR> "
			"" << std::endl;

		return ErrorCode::CreateVulkanPresentationSurfaceFailed;
	}

	ErrorCode Renderer::CreateVulkanDevice( )
	{
		uint32_t DeviceCount = 0;

		if( ( vkEnumeratePhysicalDevices( m_VulkanInstance, &DeviceCount,
			Null ) != VK_SUCCESS ) || ( DeviceCount == 0 ) )
		{
			std::cout << "[Brawl::Renderer::CreateVulkanDevice] <ERROR> "
				"Failed to enumerate physical devices" << std::endl;

			return ErrorCode::PhysicalVulkanDeviceEnumerationFailed;
		}

		std::cout << "[Brawl::Renderer:CreateVulkanDevice] <INFO> Found " <<
			DeviceCount << ( DeviceCount == 1 ? " device" : "devices" ) <<
			std::endl;

		std::vector< VkPhysicalDevice > PhysicalDevices( DeviceCount );

		if( vkEnumeratePhysicalDevices( m_VulkanInstance, &DeviceCount,
			PhysicalDevices.data( ) ) != VK_SUCCESS )
		{
			std::cout << "[Brawl::Renderer::CreateVulkanDevice] <ERROR> "
				"Failed to enumerate physical devices" << std::endl;

			return ErrorCode::PhysicalVulkanDeviceEnumerationFailed;
		}

		uint32_t GraphicsQueueFamilyIndex = UINT32_MAX;
		uint32_t PresentQueueFamilyIndex = UINT32_MAX;

		for( const auto &PhysicalDevice : PhysicalDevices )
		{
			if( this->CheckPhysicalDeviceProperties( PhysicalDevice,
				GraphicsQueueFamilyIndex, PresentQueueFamilyIndex ) == True )
			{
				m_VulkanPhysicalDevice = PhysicalDevice;
			}
		}

		if( m_VulkanPhysicalDevice == VK_NULL_HANDLE )
		{
			std::cout << "[Brawl::Renderer::CreateVulkanDevice] <ERROR> "
				"Failed to select a physical device" << std::endl;

			return ErrorCode::PhysicalVulkanDeviceSelectionFailed;
		}

		VkPhysicalDeviceProperties DeviceProperties;
		vkGetPhysicalDeviceProperties( m_VulkanPhysicalDevice,
			&DeviceProperties );

		std::cout << "[Brawl::Renderer::CreateVulkanDevice] <INFO> "
			"Using physical device \"" << DeviceProperties.deviceName <<
			"\"" << std::endl;

		std::vector< VkDeviceQueueCreateInfo > QueueCreateInfo;
		std::vector< float > QueuePriorities = { 1.0f };

		QueueCreateInfo.push_back(
			{
				VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
				Null,
				0,
				GraphicsQueueFamilyIndex,
				static_cast< uint32_t >( QueuePriorities.size( ) ),
				QueuePriorities.data( )
			} );

		if( GraphicsQueueFamilyIndex != PresentQueueFamilyIndex )
		{
			QueueCreateInfo.push_back(
				{
					VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
					Null,
					0,
					PresentQueueFamilyIndex,
					static_cast< uint32_t >( QueuePriorities.size( ) ),
					QueuePriorities.data( )
				} );
		}

		std::vector< const char * > DeviceExtensions =
		{
			VK_KHR_SWAPCHAIN_EXTENSION_NAME
		};

		VkDeviceCreateInfo DeviceCreateInfo =
		{
			VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
			Null,
			0,
			static_cast< uint32_t >( QueueCreateInfo.size( ) ),
			QueueCreateInfo.data( ),
			0,
			Null,
			static_cast< uint32_t >( DeviceExtensions.size( ) ),
			DeviceExtensions.data( ),
			Null
		};

		if( vkCreateDevice( m_VulkanPhysicalDevice, &DeviceCreateInfo, Null,
			&m_VulkanDevice ) != VK_SUCCESS )
		{
			std::cout << "[Brawl::Renderer::CreateVulkanDevice] <ERROR> "
				"Failed to create Vulkan device" << std::endl;

			return ErrorCode::CreateVulkanInstanceFailed;
		}

		m_GraphicsQueueFamilyIndex = GraphicsQueueFamilyIndex;
		m_PresentQueueFamilyIndex = PresentQueueFamilyIndex;

		return ErrorCode::Okay;
	}

	ErrorCode Renderer::LoadDeviceLevelEntryPoints( )
	{
#define VK_DEVICE_LEVEL_FUNCTION( p_Function ) \
		if( !( p_Function = ( PFN_##p_Function )vkGetDeviceProcAddr( \
			m_VulkanDevice, #p_Function ) ) ) \
		{\
			std::cout << "[Brawl::Renderer::LoadDeviceLevelEntryPoints] "\
				"<ERROR> Failed to load function: \"" << #p_Function <<\
				"\"" << std::endl;\
			return ErrorCode::LoadVulkanDeviceLevelEntryPointsFailed;\
		}\
		else\
		{\
			std::cout << "[Brawl::Renderer::LoadDeviceLevelEntryPoints] "\
				"<INFO> Loaded function: \"" << #p_Function << "\"" <<\
				std::endl;\
		}

#include <VulkanFunctions.inl>

		return ErrorCode::Okay;
	}

	ErrorCode Renderer::GetDeviceQueues( )
	{
		vkGetDeviceQueue( m_VulkanDevice, m_GraphicsQueueFamilyIndex, 0,
			&m_GraphicsQueue );
		vkGetDeviceQueue( m_VulkanDevice, m_PresentQueueFamilyIndex, 0,
			&m_PresentQueue );

		if( m_GraphicsQueue == VK_NULL_HANDLE )
		{
			std::cout << "[Brawl::Renderer::GetDeviceQueues] <ERROR> "
				"Something went wrong acquiring the graphics queue" <<
				std::endl;

			return ErrorCode::GetVulkanDeviceQueuesFailed;
		}

		if( m_PresentQueue == VK_NULL_HANDLE )
		{
			std::cout << "[Brawl::Renderer::GetDeviceQueues] <ERROR> "
				"Something went wrong acquiring the present queue" <<
				std::endl;

			return ErrorCode::GetVulkanDeviceQueuesFailed;
		}

		return ErrorCode::Okay;
	}

	ErrorCode Renderer::CreateSemaphores( )
	{
		m_ImageAvailableSemaphores.resize( kMaxFramesInFlight );
		m_RenderingFinishedSemaphores.resize( kMaxFramesInFlight );
		m_InFlightFences.resize( kMaxFramesInFlight );

		VkSemaphoreCreateInfo SemaphoreCreateInfo =
		{
			VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
			Null,
			0
		};

		VkFenceCreateInfo FenceInfo =
		{
			VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
			Null,
			VK_FENCE_CREATE_SIGNALED_BIT
		};

		for( size_t Frame = 0; Frame < kMaxFramesInFlight; ++Frame )
		{
			if( vkCreateSemaphore( m_VulkanDevice, &SemaphoreCreateInfo,
				Null, &m_ImageAvailableSemaphores[ Frame ] ) != VK_SUCCESS )
			{
				return ErrorCode::CreateVulkanSemaphoresFailed;
			}

			if( vkCreateSemaphore( m_VulkanDevice, &SemaphoreCreateInfo,
					Null, &m_RenderingFinishedSemaphores[ Frame] ) !=
					VK_SUCCESS )
			{
				return ErrorCode::CreateVulkanSemaphoresFailed;
			}

			if( vkCreateFence( m_VulkanDevice, &FenceInfo, Null,
				&m_InFlightFences[ Frame ] ) != VK_SUCCESS )
			{
				return ErrorCode::CreateVulkanSemaphoresFailed;
			}
		}

		return ErrorCode::Okay;
	}

	ErrorCode Renderer::CleanupSwapchain( )
	{
		for( auto Framebuffer : m_SwapchainFramebuffers )
		{
			vkDestroyFramebuffer( m_VulkanDevice, Framebuffer, Null );
		}

		vkFreeCommandBuffers( m_VulkanDevice, m_CommandPool,
			static_cast< uint32_t >( m_CommandBuffers.size( ) ),
			m_CommandBuffers.data( ) );

		if( m_GraphicsPipeline != VK_NULL_HANDLE )
		{
			vkDestroyPipeline( m_VulkanDevice, m_GraphicsPipeline, Null );
			m_GraphicsPipeline = VK_NULL_HANDLE;
		}

		if( m_PipelineLayout != VK_NULL_HANDLE )
		{
			vkDestroyPipelineLayout( m_VulkanDevice, m_PipelineLayout, Null );
			m_PipelineLayout = VK_NULL_HANDLE;
		}

		if( m_RenderPass != VK_NULL_HANDLE )
		{
			vkDestroyRenderPass( m_VulkanDevice, m_RenderPass, Null );
			m_RenderPass = VK_NULL_HANDLE;
		}

		for( auto ImageView : m_SwapchainImageViews )
		{
			vkDestroyImageView( m_VulkanDevice, ImageView, Null );
		}

		vkDestroySwapchainKHR( m_VulkanDevice, m_Swapchain, Null );

		return ErrorCode::Okay;
	}

	ErrorCode Renderer::RecreateSwapchain( )
	{
		vkDeviceWaitIdle( m_VulkanDevice );

		this->CleanupSwapchain( );

		this->CreateSwapchain( );
		this->CreateSwapchainImageViews( );
		this->CreateRenderPass( );
		this->CreateGraphicsPipeline( );
		this->CreateFramebuffers( );
		this->CreateCommandBuffers( );

		return ErrorCode::Okay;
	}

	ErrorCode Renderer::CreateSwapchainImageViews( )
	{
		std::cout << "Swap chain image size: " << m_SwapchainImages.size( ) << std::endl;
		m_SwapchainImageViews.resize( m_SwapchainImages.size( ) );

		for( size_t Index = 0; Index < m_SwapchainImages.size( ); ++Index )
		{
			VkImageViewCreateInfo ImageViewCreateInfo =
			{
				VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
				Null,
				0,
				m_SwapchainImages[ Index ],
				VK_IMAGE_VIEW_TYPE_2D,
				m_SwapchainImageFormat,
				// VkComponentMapping
				{
					VK_COMPONENT_SWIZZLE_IDENTITY,
					VK_COMPONENT_SWIZZLE_IDENTITY,
					VK_COMPONENT_SWIZZLE_IDENTITY,
					VK_COMPONENT_SWIZZLE_IDENTITY
				},
				// vkImageSubresourceRange
				{
					VK_IMAGE_ASPECT_COLOR_BIT,
					0,
					1,
					0,
					1
				}
			};

			if( vkCreateImageView( m_VulkanDevice, &ImageViewCreateInfo, Null,
				&m_SwapchainImageViews[ Index ] ) != VK_SUCCESS )
			{
				std::cout << "[Brawl::Renderer::CreateSwapchainImageViews] "
					"<ERROR> Failed to create image view at index: " <<
					Index << std::endl;

				return ErrorCode::CreateVulkanImageViewFailed;
			}

			std::cout << "[Brawl::Renderer::CreateSwapchainImageViews] <INFO> "
				"Created swap chain image view: " << Index << std::endl;
		}

		return ErrorCode::Okay;
	}

	ErrorCode Renderer::CreateRenderPass( )
	{
		VkAttachmentDescription ColourAttachment =
		{
			0,
			m_SwapchainImageFormat,
			VK_SAMPLE_COUNT_1_BIT,
			VK_ATTACHMENT_LOAD_OP_CLEAR,
			VK_ATTACHMENT_STORE_OP_STORE,
			VK_ATTACHMENT_LOAD_OP_DONT_CARE,
			VK_ATTACHMENT_STORE_OP_DONT_CARE,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
		};

		VkAttachmentReference ColourAttachmentReference =
		{
			0,
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
		};

		VkSubpassDescription Subpass =
		{
			0,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			0,
			Null,
			1,
			&ColourAttachmentReference,
			Null,
			Null,
			0,
			Null
		};

		VkSubpassDependency Depenency =
		{
			VK_SUBPASS_EXTERNAL,
			0,
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			0,
			VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
				VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
			0
		};

		VkRenderPassCreateInfo RenderPassInfo =
		{
			VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
			Null,
			0,
			1,
			&ColourAttachment,
			1,
			&Subpass,
			1,
			&Depenency
		};

		if( vkCreateRenderPass( m_VulkanDevice, &RenderPassInfo, Null,
			&m_RenderPass ) != VK_SUCCESS )
		{
			return ErrorCode::CreateVulkanRenderPassFailed;
		}

		return ErrorCode::Okay;
	}

	ErrorCode Renderer::CreateGraphicsPipeline( )
	{
		std::cout << "[Brawl::Renderer::CreateGraphicsPipeline] <INFO> "
			"Creating the graphics pipeline" << std::endl;

		File *pVertexShaderFile, *pFragmentShaderFile;
		ErrorCode FileError = FileSystem::Get( ).Open(
			"native:Data/Shaders/Basic.vert.spv", AccessFlags::Read,
			&pVertexShaderFile );

		if( FileError != ErrorCode::Okay )
		{
			std::cout << "[Brawl::Renderer::CreateGraphicsPipeline] <ERROR> "
				"Failed to open vertex shader file" << std::endl;

			return ErrorCode::FatalError;
		}

		FileError = FileSystem::Get( ).Open(
			"native:Data/Shaders/Basic.frag.spv", AccessFlags::Read,
			&pFragmentShaderFile );

		if( FileError != ErrorCode::Okay )
		{
			std::cout << "[Brawl::Renderer::CreateGraphicsPipeline] <ERROR> "
				"Failed to open fragment shader file" << std::endl;

			return ErrorCode::FatalError;
		}

		pVertexShaderFile->Rewind( );
		pFragmentShaderFile->Rewind( );

		std::vector< Byte > VertexShaderContents(
			pVertexShaderFile->GetSize( ) );
		uint64_t ReadBytes;
		pVertexShaderFile->ReadBytes( VertexShaderContents.data( ),
			VertexShaderContents.size( ), ReadBytes );

		VkShaderModule VertexShaderModule;
		CreateShaderModule( VertexShaderContents, VertexShaderModule );

		std::vector< Byte > FragmentShaderContents(
			pFragmentShaderFile->GetSize( ) );
		pFragmentShaderFile->ReadBytes( FragmentShaderContents.data( ),
			FragmentShaderContents.size( ), ReadBytes );

		VkShaderModule FragmentShaderModule;
		CreateShaderModule( FragmentShaderContents, FragmentShaderModule );

		VkPipelineShaderStageCreateInfo VertexShaderStageInfo =
		{
			VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			Null,
			0,
			VK_SHADER_STAGE_VERTEX_BIT,
			VertexShaderModule,
			"main",
			Null
		};

		VkPipelineShaderStageCreateInfo FragmentShaderStageInfo =
		{
			VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			Null,
			0,
			VK_SHADER_STAGE_FRAGMENT_BIT,
			FragmentShaderModule,
			"main",
			Null
		};

		VkPipelineShaderStageCreateInfo ShaderStages[ ] =
		{
			VertexShaderStageInfo,
			FragmentShaderStageInfo
		};

		VkPipelineVertexInputStateCreateInfo VertexInputStateInfo =
		{
			VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
			Null,
			0,
			0,
			Null,
			0,
			Null
		};

		VkPipelineInputAssemblyStateCreateInfo InputAssemblyStateInfo =
		{
			VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
			Null,
			0,
			VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
			VK_FALSE
		};

		VkViewport Viewport =
		{
			0.0f,
			0.0f,
			static_cast< float >( m_SwapchainExtent.width ),
			static_cast< float >( m_SwapchainExtent.height ),
			0.0f,
			1.0f
		};

		VkRect2D Scissor =
		{
			{ 0, 0 },
			m_SwapchainExtent
		};

		VkPipelineViewportStateCreateInfo ViewportStateInfo =
		{
			VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
			Null,
			0,
			1,
			&Viewport,
			1,
			&Scissor
		};

		VkPipelineRasterizationStateCreateInfo RasterisationStateInfo =
		{
			VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
			Null,
			0,
			VK_FALSE,
			VK_FALSE,
			VK_POLYGON_MODE_FILL,
			VK_CULL_MODE_BACK_BIT,
			VK_FRONT_FACE_CLOCKWISE,
			VK_FALSE,
			0.0f,
			0.0f,
			0.0f,
			1.0f
		};

		VkPipelineMultisampleStateCreateInfo MultisampleStateInfo =
		{
			VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
			Null,
			0,
			VK_SAMPLE_COUNT_1_BIT,
			VK_FALSE,
			1.0f,
			Null,
			VK_FALSE,
			VK_FALSE
		};

		VkPipelineColorBlendAttachmentState ColourBlendAttachmentState =
		{
			VK_FALSE,
			VK_BLEND_FACTOR_ONE,
			VK_BLEND_FACTOR_ZERO,
			VK_BLEND_OP_ADD,
			VK_BLEND_FACTOR_ONE,
			VK_BLEND_FACTOR_ZERO,
			VK_BLEND_OP_ADD,
			VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
			VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
		};

		VkPipelineColorBlendStateCreateInfo ColourBlendStateInfo =
		{
			VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
			Null,
			0,
			VK_FALSE,
			VK_LOGIC_OP_COPY,
			1,
			&ColourBlendAttachmentState,
			{
				0.0f,
				0.0f,
				0.0f,
				0.0f
			}
		};

		VkPipelineLayoutCreateInfo PipelineLayoutInfo =
		{
			VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
			Null,
			0,
			0,
			Null,
			0,
			Null
		};

		if( vkCreatePipelineLayout( m_VulkanDevice, &PipelineLayoutInfo, Null,
			&m_PipelineLayout ) != VK_SUCCESS )
		{
			return ErrorCode::VulkanCreatePipelineLayoutFailed;
		}

		VkGraphicsPipelineCreateInfo GraphicsPipelineInfo =
		{
			VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
			Null,
			0,
			2,
			ShaderStages,
			&VertexInputStateInfo,
			&InputAssemblyStateInfo,
			Null,
			&ViewportStateInfo,
			&RasterisationStateInfo,
			&MultisampleStateInfo,
			Null,
			&ColourBlendStateInfo,
			Null,
			m_PipelineLayout,
			m_RenderPass,
			0,
			VK_NULL_HANDLE,
			-1
		};

		if( vkCreateGraphicsPipelines( m_VulkanDevice, VK_NULL_HANDLE, 1,
			&GraphicsPipelineInfo, Null, &m_GraphicsPipeline ) != VK_SUCCESS )
		{
			return ErrorCode::VulkanCreateGraphicsPipelineFailed;
		}

		vkDestroyShaderModule( m_VulkanDevice, FragmentShaderModule, Null );
		vkDestroyShaderModule( m_VulkanDevice, VertexShaderModule, Null );

		return ErrorCode::Okay;
	}

	ErrorCode Renderer::CreateFramebuffers( )
	{
		m_SwapchainFramebuffers.resize( m_SwapchainImageViews.size( ) );

		for( size_t View = 0; View < m_SwapchainImageViews.size( ); ++View )
		{
			VkImageView Attachments [ ] =
			{
				m_SwapchainImageViews[ View ]
			};

			VkFramebufferCreateInfo FramebufferInfo =
			{
				VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
				Null,
				0,
				m_RenderPass,
				1,
				Attachments,
				m_SwapchainExtent.width,
				m_SwapchainExtent.height,
				1
			};

			if( vkCreateFramebuffer( m_VulkanDevice, &FramebufferInfo, Null,
				&m_SwapchainFramebuffers[ View ] ) != VK_SUCCESS )
			{
				return ErrorCode::VulkanCreateFramebufferFailed;
			}
		}

		return ErrorCode::Okay;
	}

	ErrorCode Renderer::CreateCommandPool( )
	{
		VkCommandPoolCreateInfo CommandPoolInfo =
		{
			VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
			Null,
			0,
			m_GraphicsQueueFamilyIndex
		};

		if( vkCreateCommandPool( m_VulkanDevice, &CommandPoolInfo, Null,
			&m_CommandPool ) != VK_SUCCESS )
		{
			return ErrorCode::VulkanCreateCommandPoolFailed;
		}

		return ErrorCode::Okay;
	}

	uint32_t Renderer::GetSwapchainImageCount(
		VkSurfaceCapabilitiesKHR &p_SurfaceCapabilities )
	{
		uint32_t ImageCount = p_SurfaceCapabilities.minImageCount + 1;

		if( ( p_SurfaceCapabilities.maxImageCount > 0 ) &&
			( ImageCount > p_SurfaceCapabilities.maxImageCount ) )
		{
			ImageCount = p_SurfaceCapabilities.maxImageCount;
		}

		return ImageCount;
	}

	VkSurfaceFormatKHR Renderer::GetSwapchainFormat(
		std::vector< VkSurfaceFormatKHR > &p_SurfaceFormats )
	{
		if( ( p_SurfaceFormats.size( ) == 1 ) &&
			( p_SurfaceFormats[ 0 ].format == VK_FORMAT_UNDEFINED ) )
		{
			return { VK_FORMAT_R8G8B8A8_UNORM,
				VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
		}

		for( auto &SurfaceFormat : p_SurfaceFormats )
		{
			if( SurfaceFormat.format == VK_FORMAT_R8G8B8A8_UNORM )
			{
				return SurfaceFormat;
			}
		}

		return p_SurfaceFormats[ 0 ];
	}

	VkExtent2D Renderer::GetSwapchainExtent(
		VkSurfaceCapabilitiesKHR &p_SurfaceCapabilities )
	{
		if( p_SurfaceCapabilities.currentExtent.width !=
			std::numeric_limits< uint32_t >::max( ) )
		{
			return p_SurfaceCapabilities.currentExtent;
		}

		// Try and get the best resolution
		VkExtent2D SwapchainExtent =
			{ m_pGameWindow->GetWidth( ), m_pGameWindow->GetHeight( ) };

		SwapchainExtent.width  = std::max(
			p_SurfaceCapabilities.minImageExtent.width,
			std::min( p_SurfaceCapabilities.maxImageExtent.width,
				SwapchainExtent.width ) );

		SwapchainExtent.height  = std::max(
			p_SurfaceCapabilities.minImageExtent.height,
			std::min( p_SurfaceCapabilities.maxImageExtent.height,
				SwapchainExtent.height ) );

		return SwapchainExtent;
	}

	VkImageUsageFlags Renderer::GetSwapchainUsageFlags(
		VkSurfaceCapabilitiesKHR &p_SurfaceCapabilities )
	{
		if( p_SurfaceCapabilities.supportedUsageFlags &
			VK_IMAGE_USAGE_TRANSFER_DST_BIT )
		{
			return VK_IMAGE_USAGE_TRANSFER_DST_BIT |
				VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		}

		std::cout << "[Brawl::Renderer::GetSwapchainUsageFlags] <ERROR> "
			"VK_IMAGE_USAGE_TRANSFER_DST_BIT is not supported by the swapchain"
			<< std::endl;

		return static_cast< VkImageUsageFlags >( -1 );
	}

	VkSurfaceTransformFlagBitsKHR Renderer::GetSwapchainTransformFlagBits(
		VkSurfaceCapabilitiesKHR &p_SurfaceCapabilities )
	{
		if( p_SurfaceCapabilities.supportedTransforms &
			VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR )
		{
			return VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
		}

		return p_SurfaceCapabilities.currentTransform;
	}

	VkPresentModeKHR Renderer::GetSwapchainPresentMode(
		std::vector< VkPresentModeKHR > &p_PresentModes )
	{
		for( auto &PresentMode : p_PresentModes )
		{
			if( PresentMode == VK_PRESENT_MODE_MAILBOX_KHR )
			{
				return PresentMode;
			}
		}

		// Runner-up
		for( auto &PresentMode : p_PresentModes )
		{
			if( PresentMode == VK_PRESENT_MODE_FIFO_KHR )
			{
				return PresentMode;
			}
		}

		std::cout << "[Brawl::Renderer::GetSwapchainPresentMode] <ERROR> "
			"No suitable present modes available" << std::endl;

		return static_cast< VkPresentModeKHR >( -1 );
	}

	Bool Renderer::CheckExtensionAvailability( const char *p_pExtensionName,
		const std::vector< VkExtensionProperties > &p_Extensions )
	{
		for( size_t Index = 0; Index < p_Extensions.size( ); ++Index )
		{
			if( strcmp( p_Extensions[ Index ].extensionName,
				p_pExtensionName ) == 0 )
			{
				return True;
			}
		}

		return False;
	}

	Bool Renderer::CheckPhysicalDeviceProperties(
		VkPhysicalDevice p_PhysicalDevice,
		uint32_t &p_GraphicsQueueFamilyIndex,
		uint32_t &p_PresentQueueFamilyIndex )
	{
		uint32_t ExtensionCount = 0;

		if( ( vkEnumerateDeviceExtensionProperties( p_PhysicalDevice, Null,
			&ExtensionCount, Null ) != VK_SUCCESS ) ||
			( ExtensionCount == 0 ) )
		{
			std::cout << "[Brawl::Renderer::CheckPhysicalDeviceProperties] "
				"<ERROR> Failed to get the extension count for the device" <<
				std::endl;

			return False;
		}

		std::vector< VkExtensionProperties > AvailableExtensions(
			ExtensionCount );

		if( vkEnumerateDeviceExtensionProperties( p_PhysicalDevice, Null,
			&ExtensionCount, AvailableExtensions.data( ) ) != VK_SUCCESS )
		{
			std::cout << "[Brawl::Renderer::CheckPhysicalDeviceProperties] "
				"<ERROR> Failed to enumerate the device's extensions" <<
				std::endl;

			return False;
		}

		std::vector< const char * > DeviceExtensions =
		{
			VK_KHR_SWAPCHAIN_EXTENSION_NAME
		};

		for( const auto &Extension : DeviceExtensions )
		{
			if( this->CheckExtensionAvailability( Extension,
				AvailableExtensions ) == False )
			{
				std::cout << "[Brawl::Renderer::CheckPhysicalDeviceProperties]"
					" <ERROR> Device extension \"" << Extension <<
					"\" is not available" << std::endl;

				return False;
			}
		}

		VkPhysicalDeviceProperties DeviceProperties;
		VkPhysicalDeviceFeatures DeviceFeatures;

		vkGetPhysicalDeviceProperties( p_PhysicalDevice, &DeviceProperties );
		vkGetPhysicalDeviceFeatures( p_PhysicalDevice, &DeviceFeatures );

		uint32_t MajorVersion =
			VK_VERSION_MAJOR( DeviceProperties.apiVersion );

		if( ( MajorVersion < 1 ) ||
			( DeviceProperties.limits.maxImageDimension2D < 4096 ) )
		{
			std::cout << "[Brawl::Renderer::CheckPhysicalDeviceProperties] "
				"<ERROR> Physical device \"" << p_PhysicalDevice <<
				"\" does not support required parameters" << std::endl;

			return False;
		}

		uint32_t QueueFamiliesCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties( p_PhysicalDevice,
			&QueueFamiliesCount, Null );

		if( QueueFamiliesCount == 0 )
		{
			std::cout << "[Brawl::Renderer::CheckPhysicalDeviceProperties] "
				"<ERROR> Physical device \"" << p_PhysicalDevice <<
				"\" does not have any queue families" << std::endl;

			return False;
		}

		std::vector< VkQueueFamilyProperties > QueueFamilyProperties(
			QueueFamiliesCount );
		std::vector< VkBool32 > QueuePresentSupport( QueueFamiliesCount );

		vkGetPhysicalDeviceQueueFamilyProperties( p_PhysicalDevice,
			&QueueFamiliesCount, QueueFamilyProperties.data( ) );

		uint32_t GraphicsQueueFamilyIndex = UINT32_MAX;
		uint32_t PresentQueueFamilyIndex = UINT32_MAX;

		for( uint32_t Index = 0; Index < QueueFamiliesCount; ++Index )
		{
			vkGetPhysicalDeviceSurfaceSupportKHR( p_PhysicalDevice,
				Index, m_PresentationSurface, &QueuePresentSupport[ Index ] );

			if( ( QueueFamilyProperties[ Index ].queueCount > 0 ) &&
				( QueueFamilyProperties[ Index ].queueFlags &
					VK_QUEUE_GRAPHICS_BIT ) )
			{
				if( GraphicsQueueFamilyIndex == UINT32_MAX )
				{
					GraphicsQueueFamilyIndex = Index;
				}

				if( QueuePresentSupport[ Index ] )
				{
					p_GraphicsQueueFamilyIndex = Index;
					p_PresentQueueFamilyIndex = Index;
					return True;
				}
			}
		}

		for( uint32_t Index = 0; Index < QueueFamiliesCount; ++Index )
		{
			if( QueuePresentSupport[ Index ] )
			{
				PresentQueueFamilyIndex = Index;
				break;
			}
		}

		if( ( PresentQueueFamilyIndex == UINT32_MAX ) ||
			( GraphicsQueueFamilyIndex == UINT32_MAX ) )
		{
			std::cout << "[Brawl::Renderer::CheckPhysicalDeviceProperties] "
				"<ERROR> Could not find queue family with required properties "
				"on physical device \"" << p_PhysicalDevice << "\"" <<
				std::endl;

			return False;
		}

		p_GraphicsQueueFamilyIndex = GraphicsQueueFamilyIndex;
		p_PresentQueueFamilyIndex = PresentQueueFamilyIndex;

		return True;
	}
	
	Bool Renderer::CheckValidationLayerSupport(
		const std::vector< const char * > &p_ValidationLayers )
	{
		uint32_t LayerCount;
		vkEnumerateInstanceLayerProperties( &LayerCount, Null );

		std::vector< VkLayerProperties > AvailableLayers( LayerCount );
		vkEnumerateInstanceLayerProperties( &LayerCount,
			AvailableLayers.data( ) );

		std::cout << "[Brawl::Renderer::CheckValidationLayerSupport] <INFO> "
			"Found " << LayerCount << " validation layers:" << std::endl;
		for( auto Layer : AvailableLayers )
		{
			std::cout << "\t" << Layer.layerName << std::endl;
		}

		for( auto LayerName : p_ValidationLayers )
		{
			Bool LayerFound = False;

			for( const auto &LayerProperties : AvailableLayers )
			{
				if( strcmp( LayerName, LayerProperties.layerName ) == 0 )
				{
					LayerFound = True;
					break;
				}
			}

			if( LayerFound == False )
			{
				return False;
			}
		}

		return True;
	}

	ErrorCode Renderer::CreateSwapchain( )
	{
		m_CanRender = False;

		if( m_VulkanDevice != VK_NULL_HANDLE )
		{
			vkDeviceWaitIdle( m_VulkanDevice );
		}

		VkSurfaceCapabilitiesKHR SurfaceCapabilities;

		if( vkGetPhysicalDeviceSurfaceCapabilitiesKHR( m_VulkanPhysicalDevice,
			m_PresentationSurface, &SurfaceCapabilities ) != VK_SUCCESS )
		{
			std::cout << "[Brawl::Renderer::CreateSwapchain] <ERROR> "
				"Unable to check the surface presentation capabilities" <<
				std::endl;

			return ErrorCode::NoVulkanDeviceSurfaceCapabilities;
		}

		uint32_t FormatCount;

		if( ( vkGetPhysicalDeviceSurfaceFormatsKHR( m_VulkanPhysicalDevice,
			m_PresentationSurface, &FormatCount, Null ) != VK_SUCCESS ) ||
			( FormatCount == 0 )  )
		{
			std::cout << "[Brawl::Renderer::CreateSwapchain] <ERROR> " <<
				"Could not get the number of surface formats" << std::endl;

			return ErrorCode::VulkanDeviceSurfaceFormatCountFailed;
		}

		std::vector< VkSurfaceFormatKHR > SurfaceFormats( FormatCount );

		if( vkGetPhysicalDeviceSurfaceFormatsKHR( m_VulkanPhysicalDevice,
			m_PresentationSurface, &FormatCount, SurfaceFormats.data( ) ) !=
				VK_SUCCESS )
		{
			std::cout << "[Brawl::Renderer::CreateSwapchain] <ERROR> " <<
				"Failed to enumerate the surface formats" << std::endl;

			return ErrorCode::VulkanDeviceSurfaceFormatEnumerationFailed;
		}

		uint32_t PresentModesCount;

		if( ( vkGetPhysicalDeviceSurfacePresentModesKHR(
				m_VulkanPhysicalDevice, m_PresentationSurface,
				&PresentModesCount, Null ) != VK_SUCCESS ) ||
			( PresentModesCount == 0 ) )
		{
			std::cout << "[Brawl::Renderer::CreateSwapchain] <ERROR> " <<
				"Could not get the number of surface presentation modes" <<
				std::endl;

			return ErrorCode::VulkanDevicePresentationModesCountFailed;
		}

		std::vector< VkPresentModeKHR > PresentModes( PresentModesCount );

		if( vkGetPhysicalDeviceSurfacePresentModesKHR(
			m_VulkanPhysicalDevice, m_PresentationSurface,
			&PresentModesCount, PresentModes.data( ) ) != VK_SUCCESS )
		{
			std::cout << "[Brawl::Renderer::CreateSwapchain] <ERROR> " <<
				"Failed to enumerate the surface presentation modes" <<
				std::endl;

			return ErrorCode::VulkanDevicePresentationModesEnumerationFailed;
		}

		uint32_t ImageCount =
			this->GetSwapchainImageCount( SurfaceCapabilities );
		VkSurfaceFormatKHR SurfaceFormat =
			this->GetSwapchainFormat( SurfaceFormats );
		VkExtent2D SurfaceExtent =
			this->GetSwapchainExtent( SurfaceCapabilities );
		VkImageUsageFlags ImageUsageFlags =
			this->GetSwapchainUsageFlags( SurfaceCapabilities );
		VkSurfaceTransformFlagBitsKHR TransformFlagBits =
			this->GetSwapchainTransformFlagBits( SurfaceCapabilities );
		VkPresentModeKHR PresentMode =
			this->GetSwapchainPresentMode( PresentModes );

		if( static_cast< int32_t >( ImageUsageFlags ) == -1 )
		{
			std::cout << "[Brawl::Renderer::CreateSwapchain] <ERROR> "
				"Usage flags incorrect" << std::endl;

			return ErrorCode::IncorrectVulkanImageUsageFlags;
		}

		if( static_cast< int32_t >( PresentMode ) == -1 )
		{
			std::cout << "[Brawl::Renderer::CreateSwapchain] <ERROR> "
				"Present mode incorrect" << std::endl;

			return ErrorCode::IncorrectVulkanPresentMode;
		}

		if( ( SurfaceExtent.width == 0 ) || ( SurfaceExtent.height == 0 ) )
		{
			std::cout << "[Brawl::CreateSwapchain] <INFO> "
				"Window minimised" << std::endl;

			return ErrorCode::Okay;
		}

		VkSwapchainCreateInfoKHR SwapchainCreateInfo =
		{
			VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
			Null,
			0,
			m_PresentationSurface,
			ImageCount,
			SurfaceFormat.format,
			SurfaceFormat.colorSpace,
			SurfaceExtent,
			1,
			ImageUsageFlags,
			VK_SHARING_MODE_EXCLUSIVE,
			0,
			Null,
			TransformFlagBits,
			VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
			PresentMode,
			VK_TRUE,
			VK_NULL_HANDLE
		};

		if( m_GraphicsQueueFamilyIndex != m_PresentQueueFamilyIndex )
		{
			// Until VK_SHARING_MODE_EXCLUSIVE is understood better,
			// VK_SHARING_MODE_CONCURRENT will do
			SwapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			SwapchainCreateInfo.queueFamilyIndexCount = 2;
			SwapchainCreateInfo.pQueueFamilyIndices =
				&m_GraphicsQueueFamilyIndex;
		}

		if( vkCreateSwapchainKHR( m_VulkanDevice, &SwapchainCreateInfo,
			Null, &m_Swapchain ) != VK_SUCCESS )
		{
			std::cout << "[Brawl::Renderer::CreateSwapchain] <ERROR> "
				"Failed to create the swapchain" << std::endl;

			return ErrorCode::CreateVulkanSwapchainFailed;
		}

		std::cout << "[Brawl::Renderer::CreateSwapchain] <INFO> "
			"Created a " << SurfaceExtent.width << "x" <<
			SurfaceExtent.height << " swapchain" << std::endl;

		vkGetSwapchainImagesKHR( m_VulkanDevice, m_Swapchain, &ImageCount,
			Null );

		std::cout << "[Brawl::Renderer::CreateSwapchain] <INFO> " <<
			ImageCount << " swapchain images acquired" << std::endl;

		m_SwapchainImages.resize( ImageCount );
		vkGetSwapchainImagesKHR( m_VulkanDevice, m_Swapchain, &ImageCount,
			m_SwapchainImages.data( ) );

		m_CanRender = True;
		m_SwapchainImageFormat = SurfaceFormat.format;
		m_SwapchainExtent = SurfaceExtent;

		return ErrorCode::Okay;
	}

	ErrorCode Renderer::CreateCommandBuffers( )
	{
		std::cout << "[Brawl::Renderer::CreateCommandBuffers] <INFO> "
			"Creating command buffers" << std::endl;

		m_CommandBuffers.resize( m_SwapchainFramebuffers.size( ) );

		VkCommandBufferAllocateInfo AllocateInfo =
		{
			VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			Null,
			m_CommandPool,
			VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			static_cast< uint32_t >( m_CommandBuffers.size( ) )
		};

		if( vkAllocateCommandBuffers( m_VulkanDevice, &AllocateInfo,
			m_CommandBuffers.data( ) ) != VK_SUCCESS )
		{
			return ErrorCode::VulkanAllocateCommandBuffersFailed;
		}

		for( size_t Buffer = 0; Buffer < m_CommandBuffers.size( ); ++Buffer )
		{
			std::cout << "Buffer: " << Buffer << std::endl;
			VkCommandBufferBeginInfo BeginInfo =
			{
				VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
				Null,
				0,
				Null
			};

			if( vkBeginCommandBuffer( m_CommandBuffers[ Buffer ],
				&BeginInfo ) != VK_SUCCESS )
			{
				return ErrorCode::VulkanBeginCommandBufferFailed;
			}

			VkClearValue ClearColour = { 0.0f, 1.0f, 0.0f, 1.0f };

			VkRenderPassBeginInfo RenderPassBeginInfo =
			{
				VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
				Null,
				m_RenderPass,
				m_SwapchainFramebuffers[ Buffer ],
				{
					{ 0, 0 },
					m_SwapchainExtent
				},
				1,
				&ClearColour
			};

			vkCmdBeginRenderPass( m_CommandBuffers[ Buffer ],
				&RenderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE );

			vkCmdBindPipeline( m_CommandBuffers[ Buffer ],
				VK_PIPELINE_BIND_POINT_GRAPHICS, m_GraphicsPipeline );

			vkCmdDraw( m_CommandBuffers[ Buffer ], 3, 1, 0, 0 );

			vkCmdEndRenderPass( m_CommandBuffers[ Buffer ] );

			if( vkEndCommandBuffer( m_CommandBuffers[ Buffer ] ) !=
				VK_SUCCESS )
			{
				return ErrorCode::VulkanEndCommandBufferFailed;
			}
		}

		return ErrorCode::Okay;
	}

	ErrorCode Renderer::CreateShaderModule( const std::vector< Byte > &p_Code,
		VkShaderModule &p_ShaderModule )
	{
		VkShaderModuleCreateInfo CreateInfo =
		{
			VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
			Null,
			0,
			p_Code.size( ),
			reinterpret_cast< const uint32_t * >( p_Code.data( ) )
		};

		if( vkCreateShaderModule( m_VulkanDevice, &CreateInfo, Null,
			&p_ShaderModule ) != VK_SUCCESS )
		{
			return ErrorCode::CreateVulkanShaderModuleFailed;
		}

		return ErrorCode::Okay;
	}

	Bool Renderer::CanRender( ) const
	{
		return m_CanRender;
	}

	ErrorCode Renderer::DrawFrame( )
	{
		vkWaitForFences( m_VulkanDevice, 1,
			&m_InFlightFences[ m_CurrentFrame ], VK_TRUE, UINT64_MAX );
		vkResetFences( m_VulkanDevice, 1,
			&m_InFlightFences[ m_CurrentFrame ] );

		uint32_t ImageIndex;
		VkResult VulkanResult = vkAcquireNextImageKHR(
			m_VulkanDevice, m_Swapchain, UINT64_MAX,
			m_ImageAvailableSemaphores[ m_CurrentFrame ], VK_NULL_HANDLE,
			&ImageIndex );

		if( VulkanResult == VK_ERROR_OUT_OF_DATE_KHR )
		{
			RecreateSwapchain( );
			return ErrorCode::Okay;
		}
		else if( VulkanResult != VK_SUCCESS &&
			VulkanResult != VK_SUBOPTIMAL_KHR )
		{
			return ErrorCode::FatalError;
		}

		VkSemaphore WaitSemaphores[ ] =
			{ m_ImageAvailableSemaphores[ m_CurrentFrame ] };
		VkPipelineStageFlags WaitStages[ ] =
			{ VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		VkSemaphore SignalSemaphores[ ] =
			{ m_RenderingFinishedSemaphores[ m_CurrentFrame ] };

		VkSubmitInfo SubmitInfo =
		{
			VK_STRUCTURE_TYPE_SUBMIT_INFO,
			Null,
			1,
			WaitSemaphores,
			WaitStages,
			1,
			&m_CommandBuffers[ ImageIndex ],
			1,
			SignalSemaphores
		};

		if( vkQueueSubmit( m_GraphicsQueue, 1, &SubmitInfo,
			m_InFlightFences[ m_CurrentFrame ] ) != VK_SUCCESS )
		{
			return ErrorCode::VulkanQueueSubmitFailed;
		}

		VkSwapchainKHR Swapchains[ ] = { m_Swapchain };

		VkPresentInfoKHR PresentInfo =
		{
			VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
			Null,
			1,
			SignalSemaphores,
			1,
			Swapchains,
			&ImageIndex,
			Null
		};

		VulkanResult = vkQueuePresentKHR( m_PresentQueue, &PresentInfo );

		if( VulkanResult == VK_ERROR_OUT_OF_DATE_KHR ||
			VulkanResult == VK_SUBOPTIMAL_KHR ||
			m_Resize == True )
		{
			m_Resize = False;
			RecreateSwapchain( );
			return ErrorCode::Okay;
		}
		else if( VulkanResult != VK_SUCCESS )
		{
			return ErrorCode::FatalError;
		}

		m_CurrentFrame = ( m_CurrentFrame + 1 ) % kMaxFramesInFlight;

		return ErrorCode::Okay;
	}

	void Renderer::TriggerResize( )
	{
		m_Resize = True;
	}
}

