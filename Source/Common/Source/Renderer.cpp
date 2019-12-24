#include <Renderer.hpp>
#include <System.hpp>
#include <VulkanFunctions.hpp>
#include <File.hpp>
#include <FileSystem.hpp>
#include <Arithmetic.hpp>

#if defined( VK_USE_PLATFORM_XCB_KHR )
#include <GameWindowXCB.hpp>
#endif // VK_USE_PLATFORM_XCB_KHR

#include <iostream>
#include <cstring>
#include <limits>
#include <chrono>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

namespace Brawl
{
	VkVertexInputBindingDescription Vertex::GetBindingDescription( )
	{
		VkVertexInputBindingDescription BindingDescription = { };
		BindingDescription.binding = 0;
		BindingDescription.stride = sizeof( Vertex );
		BindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return BindingDescription;
	}

	std::array< VkVertexInputAttributeDescription, 3 >
		Vertex::GetAttributeDescriptions( )
	{
		std::array< VkVertexInputAttributeDescription, 3 >
			AttributeDescriptions = { };

		AttributeDescriptions[ 0 ].binding = 0;
		AttributeDescriptions[ 0 ].location = 0;
		AttributeDescriptions[ 0 ].format = VK_FORMAT_R32G32B32_SFLOAT;
		AttributeDescriptions[ 0 ].offset = offsetof( Vertex, Position );

		AttributeDescriptions[ 1 ].binding = 0;
		AttributeDescriptions[ 1 ].location = 1;
		AttributeDescriptions[ 1 ].format = VK_FORMAT_R32G32B32_SFLOAT;
		AttributeDescriptions[ 1 ].offset = offsetof( Vertex, Colour );

		AttributeDescriptions[ 2 ].binding = 0;
		AttributeDescriptions[ 2 ].location = 2;
		AttributeDescriptions[ 2 ].format = VK_FORMAT_R32G32_SFLOAT;
		AttributeDescriptions[ 2 ].offset = offsetof( Vertex, UV );

		return AttributeDescriptions;
	}

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
		m_DepthImage( VK_NULL_HANDLE ),
		m_DepthImageMemory( VK_NULL_HANDLE ),
		m_DepthImageView( VK_NULL_HANDLE ),
		m_DescriptorSetLayout( VK_NULL_HANDLE ),
		m_PipelineLayout( VK_NULL_HANDLE ),
		m_GraphicsPipeline( VK_NULL_HANDLE ),
		m_RenderPass( VK_NULL_HANDLE ),
		m_CommandPool( VK_NULL_HANDLE ),
		m_DescriptorPool( VK_NULL_HANDLE ),
		m_CurrentFrame( 0 ),
		m_CanRender( False ),
		m_Resize( False ),
		m_VertexBuffer( VK_NULL_HANDLE ),
		m_VertexBufferMemory( VK_NULL_HANDLE ),
		m_IndexBuffer( VK_NULL_HANDLE ),
		m_IndexBufferMemory( VK_NULL_HANDLE ),
		m_TextureImage( VK_NULL_HANDLE ),
		m_TextureImageMemory( VK_NULL_HANDLE ),
		m_TextureImageView( VK_NULL_HANDLE ),
		m_TextureSampler( VK_NULL_HANDLE ),
		m_UniformBuffers( VK_NULL_HANDLE ),
		m_UniformBuffersMemory( VK_NULL_HANDLE )
	{
		Vertex NewVertex;

		NewVertex.Position[ 0 ] = -0.5f;
		NewVertex.Position[ 1 ] = -0.5f;
		NewVertex.Position[ 2 ] = 0.0f;
		NewVertex.Colour[ 0 ] = 1.0f;
		NewVertex.Colour[ 1 ] = 0.0f;
		NewVertex.Colour[ 2 ] = 0.0f;
		NewVertex.UV[ 0 ] = 1.0f;
		NewVertex.UV[ 1 ] = 1.0f;

		m_Vertices.push_back( NewVertex );

		NewVertex.Position[ 0 ] = 0.5f;
		NewVertex.Position[ 1 ] = -0.5f;
		NewVertex.Colour[ 0 ] = 0.0f;
		NewVertex.Colour[ 1 ] = 1.0f;
		NewVertex.Colour[ 2 ] = 0.0f;
		NewVertex.UV[ 0 ] = 0.0f;
		NewVertex.UV[ 1 ] = 1.0f;

		m_Vertices.push_back( NewVertex );

		NewVertex.Position[ 0 ] = 0.5f;
		NewVertex.Position[ 1 ] = 0.5f;
		NewVertex.Colour[ 0 ] = 0.0f;
		NewVertex.Colour[ 1 ] = 0.0f;
		NewVertex.Colour[ 2 ] = 1.0f;
		NewVertex.UV[ 0 ] = 0.0f;
		NewVertex.UV[ 1 ] = 0.0f;

		m_Vertices.push_back( NewVertex );

		NewVertex.Position[ 0 ] = -0.5f;
		NewVertex.Position[ 1 ] = 0.5f;
		NewVertex.Colour[ 0 ] = 1.0f;
		NewVertex.Colour[ 1 ] = 1.0f;
		NewVertex.Colour[ 2 ] = 1.0f;
		NewVertex.UV[ 0 ] = 1.0f;
		NewVertex.UV[ 1 ] = 0.0f;

		m_Vertices.push_back( NewVertex );

		NewVertex.Position[ 0 ] = -0.5f;
		NewVertex.Position[ 1 ] = -0.5f;
		NewVertex.Position[ 2 ] = -0.05f;
		NewVertex.Colour[ 0 ] = 1.0f;
		NewVertex.Colour[ 1 ] = 0.0f;
		NewVertex.Colour[ 2 ] = 0.0f;
		NewVertex.UV[ 0 ] = 1.0f;
		NewVertex.UV[ 1 ] = 1.0f;

		m_Vertices.push_back( NewVertex );

		NewVertex.Position[ 0 ] = 0.5f;
		NewVertex.Position[ 1 ] = -0.5f;
		NewVertex.Colour[ 0 ] = 0.0f;
		NewVertex.Colour[ 1 ] = 1.0f;
		NewVertex.Colour[ 2 ] = 0.0f;
		NewVertex.UV[ 0 ] = 0.0f;
		NewVertex.UV[ 1 ] = 1.0f;

		m_Vertices.push_back( NewVertex );

		NewVertex.Position[ 0 ] = 0.5f;
		NewVertex.Position[ 1 ] = 0.5f;
		NewVertex.Colour[ 0 ] = 0.0f;
		NewVertex.Colour[ 1 ] = 0.0f;
		NewVertex.Colour[ 2 ] = 1.0f;
		NewVertex.UV[ 0 ] = 0.0f;
		NewVertex.UV[ 1 ] = 0.0f;

		m_Vertices.push_back( NewVertex );

		NewVertex.Position[ 0 ] = -0.5f;
		NewVertex.Position[ 1 ] = 0.5f;
		NewVertex.Colour[ 0 ] = 1.0f;
		NewVertex.Colour[ 1 ] = 1.0f;
		NewVertex.Colour[ 2 ] = 1.0f;
		NewVertex.UV[ 0 ] = 1.0f;
		NewVertex.UV[ 1 ] = 0.0f;

		m_Vertices.push_back( NewVertex );

		m_Indices =
		{
			0, 1, 2, 2, 3, 0,
			4, 5, 6, 6, 7, 4
		};
	}

	Renderer::~Renderer( )
	{
		Terminate( );
	}

	ErrorCode Renderer::Initialise( GameWindow *p_pGameWindow )
	{
		if( LoadVulkanLibrary( ) != ErrorCode::Okay )
		{
			return ErrorCode::LoadVulkanLibraryFailed;
		}

		if( LoadExportedEntryPoints( ) != ErrorCode::Okay )
		{
			return ErrorCode::LoadVulkanEntryPointsFailed;
		}

		if( LoadGlobalLevelEntryPoints( ) != ErrorCode::Okay )
		{
			return ErrorCode::LoadVulkanGlobalLevelEntryPointsFailed;
		}

		if( CreateVulkanInstance( ) != ErrorCode::Okay )
		{
			return ErrorCode::CreateVulkanInstanceFailed;
		}

		if( LoadInstanceLevelEntryPoints( ) != ErrorCode::Okay )
		{
			return ErrorCode::LoadVulkanInstanceLevelEntryPointsFailed;
		}

		m_pGameWindow = p_pGameWindow;

		if( CreatePresentationSurface( ) != ErrorCode::Okay )
		{
			return ErrorCode::CreateVulkanPresentationSurfaceFailed;
		}

		if( CreateVulkanDevice( ) != ErrorCode::Okay )
		{
			return ErrorCode::CreateVulkanDeviceFailed;
		}

		if( LoadDeviceLevelEntryPoints( ) != ErrorCode::Okay )
		{
			return ErrorCode::LoadVulkanDeviceLevelEntryPointsFailed;
		}

		if( GetDeviceQueues( ) != ErrorCode::Okay )
		{
			return ErrorCode::GetVulkanDeviceQueuesFailed;
		}

		if( CreateSwapchain( ) != ErrorCode::Okay )
		{
			return ErrorCode::CreateVulkanSwapchainFailed;
		}

		if( CreateSwapchainImageViews( ) != ErrorCode::Okay )
		{
			return ErrorCode::CreateVulkanImageViewFailed;
		}

		if( CreateRenderPass( ) != ErrorCode::Okay )
		{
			return ErrorCode::CreateVulkanRenderPassFailed;
		}

		CreateDescriptorSetLayout( );

		if( CreateGraphicsPipeline( ) != ErrorCode::Okay )
		{
			return ErrorCode::CreateVulkanGraphicsPipelineFailed;
		}

		if( CreateCommandPool( ) != ErrorCode::Okay )
		{
			return ErrorCode::CreateVulkanCommandPoolFailed;
		}

		if( CreateDepthResources( ) != ErrorCode::Okay )
		{
			return ErrorCode::CreateDepthResourcesFailed;
		}

		if( CreateFramebuffers( ) != ErrorCode::Okay )
		{
			return ErrorCode::CreateVulkanFramebuffersFailed;
		}

		if( CreateTextureImage( ) != ErrorCode::Okay )
		{
			return ErrorCode::CreateTextureImageFailed;
		}

		if( CreateTextureImageView( ) != ErrorCode::Okay )
		{
			return ErrorCode::CreateTextureImageViewFailed;
		}

		if( CreateTextureSampler( ) != ErrorCode::Okay )
		{
			return ErrorCode::CreateTextureSamplerFailed;
		}

		if( CreateVertexBuffer( ) != ErrorCode::Okay )
		{
			return ErrorCode::CreateVulkanVertexBufferFailed;
		}

		if( CreateIndexBuffer( ) != ErrorCode::Okay )
		{
			return ErrorCode::CreateVulkanIndexBufferFailed;
		}

		CreateUniformBuffers( );
		CreateDescriptorPool( );
		CreateDescriptorSets( );

		if( CreateCommandBuffers( ) != ErrorCode::Okay )
		{
			return ErrorCode::CreateVulkanCommandBuffersFailed;
		}

		if( CreateSemaphores( ) != ErrorCode::Okay )
		{
			return ErrorCode::CreateVulkanSemaporesFailed;
		}

		p_pGameWindow->SetRenderer( this );

		return ErrorCode::Okay;
	}

	void Renderer::Terminate( )
	{
		vkDeviceWaitIdle( m_VulkanDevice );
		CleanupSwapchain( );

		vkDestroySampler( m_VulkanDevice, m_TextureSampler, Null );
		vkDestroyImageView( m_VulkanDevice, m_TextureImageView, Null );
		vkDestroyImage( m_VulkanDevice, m_TextureImage, Null );
		vkFreeMemory( m_VulkanDevice, m_TextureImageMemory, Null );

		vkDestroyDescriptorSetLayout( m_VulkanDevice, m_DescriptorSetLayout,
			Null );

		vkDestroyBuffer( m_VulkanDevice, m_IndexBuffer, Null );
		vkFreeMemory( m_VulkanDevice, m_IndexBufferMemory, Null );

		vkDestroyBuffer( m_VulkanDevice, m_VertexBuffer, Null );
		vkFreeMemory( m_VulkanDevice, m_VertexBufferMemory, Null );

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
#if defined ( BRAWL_BUILD_DEBUG )
			"VK_LAYER_KHRONOS_validation",
			"VK_LAYER_LUNARG_standard_validation"
#endif // BRAWL_BUILD_DEBUG
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

		std::cout << "[Brawl::Renderer::CreateVulkanInstance] <INFO> "
			"Discovered " << AvailableExtensions.size( ) << " extensions" <<
			std::endl;

		for( const auto &Extension : AvailableExtensions )
		{
			std::cout << "\t" << Extension.extensionName << std::endl;
		}

		std::vector< const char * > Extensions =
		{
#if defined( VK_USE_PLATFORM_XCB_KHR )
			VK_KHR_XCB_SURFACE_EXTENSION_NAME,
#endif // VK_USE_PLATFORM_XCB_KHR
			VK_KHR_SURFACE_EXTENSION_NAME
		};

		for( const auto &Extension : Extensions )
		{
			if( CheckExtensionAvailability( Extension,
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

#if defined( BRAWL_BUILD_DEBUG )
		InstanceCreateInfo.enabledLayerCount =
			static_cast< uint32_t >( ValidationLayers.size( ) );
		InstanceCreateInfo.ppEnabledLayerNames = ValidationLayers.data( );
		PopulateDebugMessengerCreateInfo( DebugCreateInfo );
		InstanceCreateInfo.pNext = ( VkDebugUtilsMessengerCreateInfoEXT * )&DebugCreateInfo;
#else
		InstanceCreateInfo.enabledLayerCount = 0;
		InstanceCreateInfo.ppEnabledLayerNames = Null;
		InstanceCreateInfo.pNext = Null;
#endif // BRAWL_BUILD_DEBUG

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

		std::cout << "[Brawl::Renderer::CreateVulkanDevice] <INFO> Found " <<
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
			if( CheckPhysicalDeviceProperties( PhysicalDevice,
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

		VkPhysicalDeviceFeatures DeviceFeatures = { };
		DeviceFeatures.samplerAnisotropy = VK_TRUE;

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
			&DeviceFeatures
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
		vkDestroyImageView( m_VulkanDevice, m_DepthImageView, Null );
		vkDestroyImage( m_VulkanDevice, m_DepthImage, Null );
		vkFreeMemory( m_VulkanDevice, m_DepthImageMemory, Null );

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

		for( size_t Image = 0; Image < m_SwapchainImages.size( ); ++Image )
		{
			vkDestroyBuffer( m_VulkanDevice, m_UniformBuffers[ Image ], Null );
			vkFreeMemory( m_VulkanDevice, m_UniformBuffersMemory[ Image ],
				Null );
		}

		vkDestroyDescriptorPool( m_VulkanDevice, m_DescriptorPool, Null );

		return ErrorCode::Okay;
	}

	ErrorCode Renderer::RecreateSwapchain( )
	{
		vkDeviceWaitIdle( m_VulkanDevice );

		CleanupSwapchain( );

		CreateSwapchain( );
		CreateSwapchainImageViews( );
		CreateRenderPass( );
		CreateGraphicsPipeline( );
		CreateDepthResources( );
		CreateFramebuffers( );
		CreateUniformBuffers( );
		CreateDescriptorPool( );
		CreateDescriptorSets( );
		CreateCommandBuffers( );

		return ErrorCode::Okay;
	}

	ErrorCode Renderer::CreateSwapchainImageViews( )
	{
		std::cout << "Swap chain image size: " << m_SwapchainImages.size( ) << std::endl;
		m_SwapchainImageViews.resize( m_SwapchainImages.size( ) );

		for( size_t Index = 0; Index < m_SwapchainImages.size( ); ++Index )
		{
			m_SwapchainImageViews[ Index ] = CreateImageView(
				m_SwapchainImages[ Index ], m_SwapchainImageFormat,
				VK_IMAGE_ASPECT_COLOR_BIT );

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

		VkAttachmentDescription DepthAttachment = { };
		DepthAttachment.format = FindDepthFormat( );
		DepthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		DepthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		DepthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		DepthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		DepthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		DepthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		DepthAttachment.finalLayout =
			VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentReference ColourAttachmentReference =
		{
			0,
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
		};

		VkAttachmentReference DepthAttachmentReference = { };
		DepthAttachmentReference.attachment = 1;
		DepthAttachmentReference.layout =
			VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkSubpassDescription Subpass =
		{
			0,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			0,
			Null,
			1,
			&ColourAttachmentReference,
			Null,
			&DepthAttachmentReference,
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

		std::array< VkAttachmentDescription, 2 > Attachments =
		{
			ColourAttachment, DepthAttachment
		};

		VkRenderPassCreateInfo RenderPassInfo =
		{
			VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
			Null,
			0,
			static_cast< uint32_t >( Attachments.size( ) ),
			Attachments.data( ),
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

		auto BindingDescription = Vertex::GetBindingDescription( );
		auto AttributeDescriptions = Vertex::GetAttributeDescriptions( );

		VkPipelineVertexInputStateCreateInfo VertexInputStateInfo =
		{
			VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
			Null,
			0,
			1,
			&BindingDescription,
			static_cast< uint32_t >( AttributeDescriptions.size( ) ),
			AttributeDescriptions.data( )
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
			VK_CULL_MODE_NONE,
			VK_FRONT_FACE_COUNTER_CLOCKWISE,
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

		VkPipelineDepthStencilStateCreateInfo DepthStencil = { };
		DepthStencil.sType =
			VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		DepthStencil.depthTestEnable = VK_TRUE;
		DepthStencil.depthWriteEnable = VK_TRUE;
		DepthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
		DepthStencil.depthBoundsTestEnable = VK_FALSE;
		DepthStencil.minDepthBounds = 0.0f;
		DepthStencil.maxDepthBounds = 1.0f;
		DepthStencil.stencilTestEnable = VK_FALSE;
		DepthStencil.front = { };
		DepthStencil.back = { };

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
			1,
			&m_DescriptorSetLayout,
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
			&DepthStencil,
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
			std::array< VkImageView, 2 > Attachments =
			{
				m_SwapchainImageViews[ View ],
				m_DepthImageView
			};

			VkFramebufferCreateInfo FramebufferInfo =
			{
				VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
				Null,
				0,
				m_RenderPass,
				static_cast< uint32_t >( Attachments.size( ) ),
				Attachments.data( ),
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

	ErrorCode Renderer::CreateVertexBuffer( )
	{
		VkDeviceSize BufferSize = sizeof( m_Vertices[ 0 ] ) *
			m_Vertices.size( );

		VkBuffer StagingBuffer;
		VkDeviceMemory StagingBufferMemory;

		CreateBuffer( BufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
				VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, StagingBuffer,
			StagingBufferMemory );

		void *pVertexData;
		vkMapMemory( m_VulkanDevice, StagingBufferMemory, 0, BufferSize, 0,
			&pVertexData );

		memcpy( pVertexData, m_Vertices.data( ),
			static_cast< size_t >( BufferSize ) );

		vkUnmapMemory( m_VulkanDevice, StagingBufferMemory );


		CreateBuffer( BufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
				VK_BUFFER_USAGE_TRANSFER_DST_BIT, 
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_VertexBuffer,
			m_VertexBufferMemory );

		CopyBuffer( StagingBuffer, m_VertexBuffer, BufferSize );

		vkDestroyBuffer( m_VulkanDevice, StagingBuffer, Null );
		vkFreeMemory( m_VulkanDevice, StagingBufferMemory, Null );

		return ErrorCode::Okay;
	}

	ErrorCode Renderer::CreateIndexBuffer( )
	{
		VkDeviceSize BufferSize = sizeof( m_Indices[ 0 ] ) * m_Indices.size( );

		VkBuffer StagingBuffer;
		VkDeviceMemory StagingBufferMemory;

		CreateBuffer( BufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
				VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			StagingBuffer, StagingBufferMemory );

		void *pIndexData;

		vkMapMemory( m_VulkanDevice, StagingBufferMemory, 0, BufferSize, 0,
			&pIndexData );

		memcpy( pIndexData, m_Indices.data( ),
			static_cast< size_t >( BufferSize ) );

		vkUnmapMemory( m_VulkanDevice, StagingBufferMemory );

		CreateBuffer( BufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT |
				VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_IndexBuffer,
			m_IndexBufferMemory );

		CopyBuffer( StagingBuffer, m_IndexBuffer, BufferSize );
		
		vkDestroyBuffer( m_VulkanDevice, StagingBuffer, Null );
		vkFreeMemory( m_VulkanDevice, StagingBufferMemory, Null );

		return ErrorCode::Okay;
	}

	ErrorCode Renderer::CreateBuffer( VkDeviceSize p_Size,
		VkBufferUsageFlags p_Usage, VkMemoryPropertyFlags p_Properties,
		VkBuffer &p_Buffer, VkDeviceMemory &p_BufferMemory )
	{
		VkBufferCreateInfo BufferCreateInfo = { };
		BufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		BufferCreateInfo.size = p_Size;
		BufferCreateInfo.usage = p_Usage;
		BufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if( vkCreateBuffer( m_VulkanDevice, &BufferCreateInfo, Null,
			&p_Buffer ) != VK_SUCCESS )
		{
			std::cout << "[Brawl::Renderer::VertexBuffer] <ERROR> "
				"Call to vkCreateBuffer failed" << std::endl;

			return ErrorCode::VulkanCreateBufferFailed;
		}

		VkMemoryRequirements MemoryRequirements;
		vkGetBufferMemoryRequirements( m_VulkanDevice, p_Buffer,
			&MemoryRequirements );

		VkMemoryAllocateInfo AllocateInfo = { };
		AllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		AllocateInfo.allocationSize = MemoryRequirements.size;
		AllocateInfo.memoryTypeIndex = FindMemoryType(
			MemoryRequirements.memoryTypeBits, p_Properties );
		// memoryTypeIndex should be verified it is valid!!

		if( vkAllocateMemory( m_VulkanDevice, &AllocateInfo, Null,
			&p_BufferMemory ) != VK_SUCCESS )
		{
			std::cout << "[Brawl::Renderer::VertexBuffer] <ERROR> "
				"Failed to alocate memory" << std::endl;

			return ErrorCode::VulkanAllocateMemoryFailed;
		}

		vkBindBufferMemory( m_VulkanDevice, p_Buffer, p_BufferMemory, 0 );

		return ErrorCode::Okay;
	}

	ErrorCode Renderer::CopyBuffer( VkBuffer p_Source, VkBuffer p_Destination,
			VkDeviceSize p_Size )
	{

		VkCommandBuffer CommandBuffer = BeginSingleTimeCommands( );

		VkBufferCopy CopyRegion = { };
		CopyRegion.srcOffset = 0;
		CopyRegion.dstOffset = 0;
		CopyRegion.size = p_Size;

		vkCmdCopyBuffer( CommandBuffer, p_Source, p_Destination, 1,
			&CopyRegion );

		EndSingleTimeCommands( CommandBuffer );

		return ErrorCode::Okay;
	}

	ErrorCode Renderer::CreateDescriptorSetLayout( )
	{
		VkDescriptorSetLayoutBinding UniformLayoutBinding = { };
		UniformLayoutBinding.binding = 0;
		UniformLayoutBinding.descriptorCount = 1;
		UniformLayoutBinding.descriptorType =
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		UniformLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		UniformLayoutBinding.pImmutableSamplers = Null;

		VkDescriptorSetLayoutBinding SamplerLayoutBinding = { };
		SamplerLayoutBinding.binding = 1;
		SamplerLayoutBinding.descriptorCount = 1;
		SamplerLayoutBinding.descriptorType =
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		SamplerLayoutBinding.pImmutableSamplers = Null;
		SamplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		std::array< VkDescriptorSetLayoutBinding, 2 > Bindings =
		{
			UniformLayoutBinding,
			SamplerLayoutBinding
		};

		VkDescriptorSetLayoutCreateInfo LayoutCreateInfo = { };
		LayoutCreateInfo.sType =
			VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		LayoutCreateInfo.bindingCount =
			static_cast< uint32_t >( Bindings.size( ) );
		LayoutCreateInfo.pBindings = Bindings.data( );

		if( vkCreateDescriptorSetLayout( m_VulkanDevice, &LayoutCreateInfo,
			Null, &m_DescriptorSetLayout ) != VK_SUCCESS )
		{
			std::cout << "[Brawl::Renderer::CreateDescriptorSetLayout] "
				"<ERROR> Failed to create descriptor set layout" << std::endl;

			return ErrorCode::CreateVulkanDescriptorSetLayoutFailed;
		}



		return ErrorCode::Okay;
	}

	ErrorCode Renderer::CreateUniformBuffers( )
	{
		VkDeviceSize BufferSize = sizeof( UniformBufferObject );

		m_UniformBuffers.resize( m_SwapchainImages.size( ) );
		m_UniformBuffersMemory.resize( m_SwapchainImages.size( ) );

		for( size_t Buffer = 0; Buffer < m_SwapchainImages.size( ); ++Buffer )
		{
			CreateBuffer( BufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
					VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				m_UniformBuffers[ Buffer ], m_UniformBuffersMemory[ Buffer ] );
		}

		return ErrorCode::Okay;
	}

	ErrorCode Renderer::UpdateUniformBuffer( uint32_t p_CurrentImage )
	{
		static auto StartTime = std::chrono::high_resolution_clock::now( );

		auto CurrentTime = std::chrono::high_resolution_clock::now( );
		float Time =
			std::chrono::duration< float, std::chrono::seconds::period >(
				CurrentTime - StartTime ).count( );

		UniformBufferObject UBO = { };

		UBO.Model.Rotate( Time * ToRadians( 90.0f ), Vector4( 0.0f, 1.0f, 0.0f, 0.0f ) );
		UBO.Projection.CreatePerspectiveFOV( ToRadians( 45.0f ),
			static_cast< float >( m_SwapchainExtent.width ) / 
				static_cast< float >( m_SwapchainExtent.height ),
			0.1f, 10.0f );
		UBO.View.CreateViewLookAt( Vector4( 2.0f, 2.0f, 2.0f, 1.0f ),
			Vector4( 0.0f, 0.0f, 0.0f, 1.0f ),
			Vector4( 0.0f, 1.0f, 0.0f, 1.0f ) );

		void *pUBOData;

		vkMapMemory( m_VulkanDevice, m_UniformBuffersMemory[ p_CurrentImage ],
			0, sizeof( UBO ), 0, &pUBOData );

		memcpy( pUBOData, &UBO, sizeof( UBO ) );

		vkUnmapMemory( m_VulkanDevice,
			m_UniformBuffersMemory[ p_CurrentImage ] );

		return ErrorCode::Okay;
	}

	ErrorCode Renderer::CreateDescriptorPool( )
	{
		std::array< VkDescriptorPoolSize, 2 > PoolSizes = { };
		PoolSizes[ 0 ].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		PoolSizes[ 0 ].descriptorCount =
			static_cast< uint32_t >( m_SwapchainImages.size( ) );
		PoolSizes[ 1 ].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		PoolSizes[ 1 ].descriptorCount =
			static_cast< uint32_t >( m_SwapchainImages.size( ) );

		VkDescriptorPoolCreateInfo DescriptorPoolCreateInfo = { };
		DescriptorPoolCreateInfo.sType =
			VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		DescriptorPoolCreateInfo.poolSizeCount =
			static_cast< uint32_t >( PoolSizes.size( ) );
		DescriptorPoolCreateInfo.pPoolSizes = PoolSizes.data( );
		DescriptorPoolCreateInfo.maxSets = static_cast< uint32_t >(
			m_SwapchainImages.size( ) );

		if( vkCreateDescriptorPool( m_VulkanDevice, &DescriptorPoolCreateInfo,
			Null, &m_DescriptorPool ) != VK_SUCCESS )
		{
			std::cout << "[Brawl::Renderer::CreateDescriptorPool] <ERROR> "
				"Failed to create a descriptor pool" << std::endl;

			return ErrorCode::FatalError;
		}

		return ErrorCode::Okay;
	}

	ErrorCode Renderer::CreateDescriptorSets( )
	{
		std::vector< VkDescriptorSetLayout > Layouts(
			m_SwapchainImages.size( ), m_DescriptorSetLayout );

		VkDescriptorSetAllocateInfo AllocateInfo = { };
		AllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		AllocateInfo.descriptorPool = m_DescriptorPool;
		AllocateInfo.descriptorSetCount = static_cast< uint32_t >(
			m_SwapchainImages.size( ) );
		AllocateInfo.pSetLayouts = Layouts.data( );

		m_DescriptorSets.resize( m_SwapchainImages.size( ) );

		if( vkAllocateDescriptorSets( m_VulkanDevice, &AllocateInfo,
			m_DescriptorSets.data( ) ) != VK_SUCCESS )
		{
			std::cout << "[Brawl::Renderer::CreateDescriptorSets] <ERROR> "
				"Failed to allocate descriptor sets" << std::endl;

			return ErrorCode::FatalError;
		}

		for( uint32_t Image = 0; Image < m_SwapchainImages.size( ); ++Image )
		{
			VkDescriptorBufferInfo BufferInfo = { };
			BufferInfo.buffer = m_UniformBuffers[ Image ];
			BufferInfo.offset = 0;
			BufferInfo.range = sizeof( UniformBufferObject );

			VkDescriptorImageInfo ImageInfo = { };
			ImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			ImageInfo.imageView = m_TextureImageView;
			ImageInfo.sampler = m_TextureSampler;

			std::array< VkWriteDescriptorSet, 2 > WriteDescriptors = { };

			WriteDescriptors[ 0 ].sType =
				VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			WriteDescriptors[ 0 ].dstSet = m_DescriptorSets[ Image ];
			WriteDescriptors[ 0 ].dstBinding = 0;
			WriteDescriptors[ 0 ].dstArrayElement = 0;
			WriteDescriptors[ 0 ].descriptorType =
				VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			WriteDescriptors[ 0 ].descriptorCount = 1;
			WriteDescriptors[ 0 ].pBufferInfo = &BufferInfo;
			WriteDescriptors[ 0 ].pImageInfo = Null;
			WriteDescriptors[ 0 ].pTexelBufferView = Null;

			WriteDescriptors[ 1 ].sType =
				VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			WriteDescriptors[ 1 ].dstSet = m_DescriptorSets[ Image ];
			WriteDescriptors[ 1 ].dstBinding = 1;
			WriteDescriptors[ 1 ].dstArrayElement = 0;
			WriteDescriptors[ 1 ].descriptorType =
				VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			WriteDescriptors[ 1 ].descriptorCount = 1;
			WriteDescriptors[ 1 ].pBufferInfo = Null;
			WriteDescriptors[ 1 ].pImageInfo = &ImageInfo;
			WriteDescriptors[ 1 ].pTexelBufferView = Null;

			vkUpdateDescriptorSets( m_VulkanDevice,
				static_cast< uint32_t >( WriteDescriptors.size( ) ),
				WriteDescriptors.data( ), 0, Null );
		}

		return ErrorCode::Okay;
	}

	ErrorCode Renderer::CreateTextureImage( )
	{
		SInt32 Width, Height, ChannelCount;

		stbi_uc *pPixels = stbi_load( "Data/Textures/512.png", &Width, &Height,
			&ChannelCount, STBI_rgb_alpha );

		if( pPixels == Null )
		{
			std::cout << "[Brawl::Renderer::CreateTextureImage] <ERROR> "
				"Failed to open texture image: Data/Textures/512.png"
				<< std::endl;

			return ErrorCode::TextureFileInvalid;
		}

		VkDeviceSize TextureSize = Width * Height * 4;

		VkBuffer StagingBuffer;
		VkDeviceMemory StagingBufferMemory;

		CreateBuffer( TextureSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
				VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, StagingBuffer,
			StagingBufferMemory );

		void *pData;
		vkMapMemory( m_VulkanDevice, StagingBufferMemory, 0, TextureSize, 0,
			&pData );
		memcpy( pData, pPixels, static_cast< size_t >( TextureSize ) );
		vkUnmapMemory( m_VulkanDevice, StagingBufferMemory );

		stbi_image_free( pPixels );

		CreateImage( Width, Height, VK_FORMAT_R8G8B8A8_UNORM,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_TRANSFER_DST_BIT |
				VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_TextureImage,
			m_TextureImageMemory );

		/*VkImageCreateInfo ImageInfo = { };
		ImageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		ImageInfo.imageType = VK_IMAGE_TYPE_2D;
		ImageInfo.extent.width = static_cast< uint32_t >( Width );
		ImageInfo.extent.height = static_cast< uint32_t >( Height );
		ImageInfo.extent.depth = 1;
		ImageInfo.mipLevels = 1;
		ImageInfo.arrayLayers = 1;
		ImageInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
		ImageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		ImageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		ImageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT |
			VK_IMAGE_USAGE_SAMPLED_BIT;
		ImageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		ImageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		ImageInfo.flags = 0;

		if( vkCreateImage( m_VulkanDevice, &ImageInfo, Null, &m_TextureImage )
			!= VK_SUCCESS )
		{
			std::cout << "[Brawl::Renderer::CreateTextureImage] <ERROR> "
				"Failed to create iamge: Data/Textures/512.png"
				<< std::endl;

			return ErrorCode::VulkanCreateImageFailed;
		}

		VkMemoryRequirements MemoryRequirements;
		vkGetImageMemoryRequirements( m_VulkanDevice, m_TextureImage,
			&MemoryRequirements );

		VkMemoryAllocateInfo AllocateInfo = { };
		AllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		AllocateInfo.allocationSize = MemoryRequirements.size;
		AllocateInfo.memoryTypeIndex = FindMemoryType(
			MemoryRequirements.memoryTypeBits,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT );

		if( vkAllocateMemory( m_VulkanDevice, &AllocateInfo, Null,
			&m_TextureImageMemory ) != VK_SUCCESS )
		{
			std::cout << "[Brawl::Renderer::CreateTextureImage] <ERROR> "
				"Failed to allocate memory for: Data/Textures/512.png"
				<< std::endl;

			return ErrorCode::VulkanAllocateMemoryFailed;
		}

		vkBindImageMemory( m_VulkanDevice, m_TextureImage,
			m_TextureImageMemory, 0 );*/

		TransitionImageLayout( m_TextureImage, VK_FORMAT_R8G8B8A8_UNORM,
			VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL );
		CopyBufferToImage( StagingBuffer, m_TextureImage,
			static_cast< UInt32 >( Width ), static_cast< UInt32 >( Height ) );
		TransitionImageLayout( m_TextureImage, VK_FORMAT_R8G8B8A8_UNORM,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL );

		vkDestroyBuffer( m_VulkanDevice, StagingBuffer, Null );
		vkFreeMemory( m_VulkanDevice, StagingBufferMemory, Null );

		return ErrorCode::Okay;
	}

	ErrorCode Renderer::CreateTextureImageView( )
	{
		m_TextureImageView = CreateImageView( m_TextureImage,
			VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT );

		return ErrorCode::Okay;
	}

	ErrorCode Renderer::CreateTextureSampler( )
	{
		VkSamplerCreateInfo SamplerInfo = { };
		SamplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		SamplerInfo.magFilter = VK_FILTER_LINEAR;
		SamplerInfo.minFilter = VK_FILTER_LINEAR;
		SamplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
		SamplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
		SamplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
		SamplerInfo.anisotropyEnable = VK_TRUE;
		SamplerInfo.maxAnisotropy = 16;
		SamplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		SamplerInfo.unnormalizedCoordinates = VK_FALSE;
		SamplerInfo.compareEnable = VK_FALSE;
		SamplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
		SamplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		SamplerInfo.mipLodBias = 0.0f;
		SamplerInfo.minLod = 0.0f;
		SamplerInfo.maxLod = 0.0f;

		if( vkCreateSampler( m_VulkanDevice, &SamplerInfo, Null,
			&m_TextureSampler ) != VK_SUCCESS )
		{
			return ErrorCode::VulkanCreateTextureSamplerFailed;
		}

		return ErrorCode::Okay;
	}

	ErrorCode Renderer::CreateDepthResources( )
	{
		VkFormat DepthFormat = FindDepthFormat( );

		CreateImage( m_SwapchainExtent.width, m_SwapchainExtent.height,
			DepthFormat, VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_DepthImage,
			m_DepthImageMemory );

		m_DepthImageView = CreateImageView( m_DepthImage, DepthFormat,
			VK_IMAGE_ASPECT_DEPTH_BIT );

		TransitionImageLayout( m_DepthImage, DepthFormat,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL );

		return ErrorCode::Okay;
	}

	ErrorCode Renderer::TransitionImageLayout( VkImage p_Image,
		VkFormat p_Format, VkImageLayout p_OldLayout,
		VkImageLayout p_NewLayout )
	{
		VkCommandBuffer CommandBuffer = BeginSingleTimeCommands( );

		VkImageMemoryBarrier Barrier = { };
		Barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		Barrier.oldLayout = p_OldLayout;
		Barrier.newLayout = p_NewLayout;
		Barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		Barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		Barrier.image = p_Image;
		Barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		Barrier.subresourceRange.baseMipLevel = 0;
		Barrier.subresourceRange.levelCount = 1;
		Barrier.subresourceRange.baseArrayLayer = 0;
		Barrier.subresourceRange.layerCount = 1;
		Barrier.srcAccessMask = 0;
		Barrier.dstAccessMask = 0;

		if( p_NewLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL )
		{
			Barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

			if( HasStencilComponent( p_Format ) )
			{
				Barrier.subresourceRange.aspectMask |=
					VK_IMAGE_ASPECT_STENCIL_BIT;
			}
		}
		else
		{
			Barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		}

		VkPipelineStageFlags SourceStage, DestinationStage;

		if( p_OldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
			p_NewLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL )
		{
			Barrier.srcAccessMask = 0;
			Barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			SourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			DestinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		else if( p_OldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
			p_NewLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL )
		{
			Barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			Barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			SourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			DestinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
		else if( p_OldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
			p_NewLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL )
		{
			Barrier.srcAccessMask = 0;
			Barrier.dstAccessMask =
				VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
				VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

			SourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			DestinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		}
		else
		{
			std::cout << "[Brawl::Renderer::TransitionImageLayout] <ERROR> "
				"Unsupported layout transition" << std::endl;

			return ErrorCode::VulkanLayoutTransitionUnsupported;
		}

		vkCmdPipelineBarrier( CommandBuffer, SourceStage, DestinationStage,
			0,
			0, Null,
			0, Null,
			1, &Barrier );

		EndSingleTimeCommands( CommandBuffer );

		return ErrorCode::Okay;
	}

	ErrorCode Renderer::CopyBufferToImage( VkBuffer p_Buffer, VkImage p_Image,
		UInt32 p_Width, UInt32 p_Height )
	{
		VkCommandBuffer CommandBuffer = BeginSingleTimeCommands( );

		VkBufferImageCopy Region = { };
		Region.bufferOffset = 0;
		Region.bufferRowLength = 0;
		Region.bufferImageHeight = 0;
		Region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		Region.imageSubresource.mipLevel = 0;
		Region.imageSubresource.baseArrayLayer = 0;
		Region.imageSubresource.layerCount = 1;
		Region.imageOffset = { 0, 0, 0 };
		Region.imageExtent = { p_Width, p_Height, 1 };

		vkCmdCopyBufferToImage( CommandBuffer, p_Buffer, p_Image,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &Region );

		EndSingleTimeCommands( CommandBuffer );

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

	VkCommandBuffer Renderer::BeginSingleTimeCommands( )
	{
		VkCommandBufferAllocateInfo AllocateInfo = { };
		AllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		AllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		AllocateInfo.commandPool = m_CommandPool;
		AllocateInfo.commandBufferCount = 1;

		VkCommandBuffer CommandBuffer;
		vkAllocateCommandBuffers( m_VulkanDevice, &AllocateInfo,
			&CommandBuffer );

		VkCommandBufferBeginInfo BeginInfo = { };
		BeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		BeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer( CommandBuffer, &BeginInfo );

		return CommandBuffer;
	}

	void Renderer::EndSingleTimeCommands( VkCommandBuffer p_CommandBuffer )
	{
		vkEndCommandBuffer( p_CommandBuffer );

		VkSubmitInfo SubmitInfo = { };
		SubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		SubmitInfo.commandBufferCount = 1;
		SubmitInfo.pCommandBuffers = &p_CommandBuffer;

		vkQueueSubmit( m_GraphicsQueue, 1, &SubmitInfo, VK_NULL_HANDLE );
		vkQueueWaitIdle( m_GraphicsQueue );

		vkFreeCommandBuffers( m_VulkanDevice, m_CommandPool, 1,
			&p_CommandBuffer );
	}

	VkFormat Renderer::FindSupportedFormat(
		const std::vector< VkFormat > &p_Candidates,
		VkImageTiling p_Tiling, VkFormatFeatureFlags p_Flags )
	{
		for( auto Format : p_Candidates )
		{
			VkFormatProperties Properties;
			vkGetPhysicalDeviceFormatProperties( m_VulkanPhysicalDevice,
				Format, &Properties );

			if( p_Tiling == VK_IMAGE_TILING_LINEAR &&
				( Properties.linearTilingFeatures & p_Flags ) == p_Flags )
			{
				return Format;
			}
			else if( p_Tiling == VK_IMAGE_TILING_OPTIMAL &&
				( Properties.optimalTilingFeatures & p_Flags ) == p_Flags )
			{
				return Format;
			}
		}

		return static_cast< VkFormat >( -1 );
	}

	VkFormat Renderer::FindDepthFormat( )
	{
		return FindSupportedFormat(
			{
				VK_FORMAT_D32_SFLOAT,
				VK_FORMAT_D32_SFLOAT_S8_UINT,
				VK_FORMAT_D24_UNORM_S8_UINT
			},
			VK_IMAGE_TILING_OPTIMAL,
			VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT );
	}

	Bool Renderer::HasStencilComponent( VkFormat p_Format )
	{
		return	( p_Format == VK_FORMAT_D32_SFLOAT_S8_UINT ) ||
				( p_Format == VK_FORMAT_D24_UNORM_S8_UINT );
	}

	VkImageView Renderer::CreateImageView( VkImage p_Image, VkFormat p_Format,
		VkImageAspectFlags p_AspectFlags )
	{

		VkImageViewCreateInfo ImageViewCreateInfo = { };

		ImageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		ImageViewCreateInfo.image = p_Image;
		ImageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		ImageViewCreateInfo.format = p_Format;
		ImageViewCreateInfo.subresourceRange.aspectMask = p_AspectFlags;
		ImageViewCreateInfo.subresourceRange.baseMipLevel = 0;
		ImageViewCreateInfo.subresourceRange.levelCount = 1;
		ImageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
		ImageViewCreateInfo.subresourceRange.layerCount = 1;

		VkImageView ImageView;

		if( vkCreateImageView( m_VulkanDevice, &ImageViewCreateInfo, Null,
			&ImageView ) != VK_SUCCESS )
		{
			std::cout << "[Brawl::Renderer::CreateImageView] <ERROR> "
				"Failed to create an image view" << std::endl;

			return VK_NULL_HANDLE;
		}

		return ImageView;
	}

	ErrorCode Renderer::CreateImage( UInt32 p_Width, UInt32 p_Height,
		VkFormat p_Format, VkImageTiling p_Tiling,
		VkImageUsageFlags p_Usage,
		VkMemoryPropertyFlags p_MemoryProperties, VkImage &p_Image,
		VkDeviceMemory &p_Memory )
	{
		VkImageCreateInfo ImageInfo = { };
		ImageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		ImageInfo.imageType = VK_IMAGE_TYPE_2D;
		ImageInfo.extent.width =  p_Width;
		ImageInfo.extent.height = p_Height;
		ImageInfo.extent.depth = 1;
		ImageInfo.mipLevels = 1;
		ImageInfo.arrayLayers = 1;
		ImageInfo.format = p_Format;
		ImageInfo.tiling = p_Tiling;
		ImageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		ImageInfo.usage = p_Usage;
		ImageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		ImageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		ImageInfo.flags = 0;

		if( vkCreateImage( m_VulkanDevice, &ImageInfo, Null, &p_Image )
			!= VK_SUCCESS )
		{
			std::cout << "[Brawl::Renderer::CreateTextureImage] <ERROR> "
				"Failed to create iamge: Data/Textures/512.png"
				<< std::endl;

			return ErrorCode::VulkanCreateImageFailed;
		}

		VkMemoryRequirements MemoryRequirements;
		vkGetImageMemoryRequirements( m_VulkanDevice, p_Image,
			&MemoryRequirements );

		VkMemoryAllocateInfo AllocateInfo = { };
		AllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		AllocateInfo.allocationSize = MemoryRequirements.size;
		AllocateInfo.memoryTypeIndex = FindMemoryType(
			MemoryRequirements.memoryTypeBits,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT );

		if( vkAllocateMemory( m_VulkanDevice, &AllocateInfo, Null, &p_Memory )
			!= VK_SUCCESS )
		{
			std::cout << "[Brawl::Renderer::CreateTextureImage] <ERROR> "
				"Failed to allocate memory for: Data/Textures/512.png"
				<< std::endl;

			return ErrorCode::VulkanAllocateMemoryFailed;
		}

		vkBindImageMemory( m_VulkanDevice, p_Image, p_Memory, 0 );

		return ErrorCode::Okay;
	}

	uint32_t Renderer::FindMemoryType( uint32_t p_TypeFilter,
			VkMemoryPropertyFlags p_Properties )
	{
		VkPhysicalDeviceMemoryProperties MemoryProperties;
		vkGetPhysicalDeviceMemoryProperties( m_VulkanPhysicalDevice, 
			&MemoryProperties );

		std::cout << "[Brawl::Renderer::FindMemoryType] <INFO> "
			"Found " << MemoryProperties.memoryTypeCount << " memory type(s)"
			<< std::endl;

		for( uint32_t Index = 0; Index < MemoryProperties.memoryTypeCount;
			++Index )
		{
			if( ( p_TypeFilter & ( 1 << Index ) ) &&
				( MemoryProperties.memoryTypes[ Index ].propertyFlags &
					p_Properties ) == p_Properties )
			{
				return Index;
			}
		}

		std::cout << "[Brawl::Renderer::FindMemoryType] <ERROR> "
			"Unable to find a suitable memory type" << std::endl;

		return std::numeric_limits< uint32_t >::max( );
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
			if( CheckExtensionAvailability( Extension,
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

		if( DeviceFeatures.samplerAnisotropy == VK_FALSE )
		{
			std::cout << "[Brawl::Renderer::CheckPhysicalDeviceProperties] "
				"<ERROR> Anisotropic filtering not supported on device \"" <<
				p_PhysicalDevice << "\"" << std::endl;

			return False;
		}

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
			GetSwapchainImageCount( SurfaceCapabilities );
		VkSurfaceFormatKHR SurfaceFormat =
			GetSwapchainFormat( SurfaceFormats );
		VkExtent2D SurfaceExtent =
			GetSwapchainExtent( SurfaceCapabilities );
		VkImageUsageFlags ImageUsageFlags =
			GetSwapchainUsageFlags( SurfaceCapabilities );
		VkSurfaceTransformFlagBitsKHR TransformFlagBits =
			GetSwapchainTransformFlagBits( SurfaceCapabilities );
		VkPresentModeKHR PresentMode =
			GetSwapchainPresentMode( PresentModes );

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

			std::array< VkClearValue, 2 > ClearValues = { };
			ClearValues[ 0 ].color =
				{ 0.0f, 17.0f / 255.0f, 43.0f / 255.0f, 1.0f };
			ClearValues[ 1 ].depthStencil = { 1.0f, 0 };

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
				static_cast< uint32_t >( ClearValues.size( ) ),
				ClearValues.data( )
			};

			vkCmdBeginRenderPass( m_CommandBuffers[ Buffer ],
				&RenderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE );

			vkCmdBindPipeline( m_CommandBuffers[ Buffer ],
				VK_PIPELINE_BIND_POINT_GRAPHICS, m_GraphicsPipeline );

			VkBuffer VertexBuffers[ ] = { m_VertexBuffer };
			VkDeviceSize Offsets[ ] = { 0 };
			vkCmdBindVertexBuffers( m_CommandBuffers[ Buffer ], 0, 1,
				VertexBuffers, Offsets );
			vkCmdBindIndexBuffer( m_CommandBuffers[ Buffer ], m_IndexBuffer, 0,
				VK_INDEX_TYPE_UINT16 );
			vkCmdBindDescriptorSets( m_CommandBuffers[ Buffer ],
				VK_PIPELINE_BIND_POINT_GRAPHICS, m_PipelineLayout, 0, 1,
				&m_DescriptorSets[ Buffer ], 0, Null );

			vkCmdDrawIndexed( m_CommandBuffers[ Buffer ],
				static_cast< uint32_t >( m_Indices.size( ) ), 1, 0, 0, 0 );
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

		UpdateUniformBuffer( ImageIndex );

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

