#ifndef __BRAWL_RENDERER_HPP__
#define __BRAWL_RENDERER_HPP__

#include <DataTypes.hpp>
#include <vulkan/vulkan.h>

#include <vector>
#include <array>

namespace Brawl
{
	class GameWindow;
	class GameWindowData;

	struct Vertex
	{
		float	Position[ 2 ];
		float	Colour[ 3 ];

		static VkVertexInputBindingDescription GetBindingDescription( );
		static std::array< VkVertexInputAttributeDescription, 2 >
			GetAttributeDescriptions( );
	};

	class Renderer
	{
	public:
		Renderer( );
		~Renderer( );

		ErrorCode Initialise( GameWindow *p_pGameWindow );

		void Terminate( );

		ErrorCode CreateSwapchain( );
		ErrorCode CreateCommandBuffers( );
		ErrorCode CreateShaderModule( const std::vector< Byte > &p_Code,
			VkShaderModule &p_ShaderModule );

		Bool CanRender( ) const;

		ErrorCode DrawFrame( );

		void TriggerResize( );

	private:
		Renderer( const Renderer &p_Other );
		Renderer &operator=( const Renderer &p_Other );

		ErrorCode LoadVulkanLibrary( );
		ErrorCode LoadExportedEntryPoints( );
		ErrorCode LoadGlobalLevelEntryPoints( );
		ErrorCode CreateVulkanInstance( );
		ErrorCode LoadInstanceLevelEntryPoints( );
		ErrorCode CreatePresentationSurface( );
		ErrorCode CreateVulkanDevice( );
		ErrorCode LoadDeviceLevelEntryPoints( );
		ErrorCode GetDeviceQueues( );
		ErrorCode CreateSemaphores( );
		ErrorCode CleanupSwapchain( );
		ErrorCode RecreateSwapchain( );
		ErrorCode CreateSwapchainImageViews( );
		ErrorCode CreateRenderPass( );
		ErrorCode CreateGraphicsPipeline( );
		ErrorCode CreateFramebuffers( );
		ErrorCode CreateCommandPool( );
		ErrorCode CreateVertexBuffer( );
		ErrorCode CreateBuffer( VkDeviceSize p_Size,
			VkBufferUsageFlags p_Usage, VkMemoryPropertyFlags p_Properties,
			VkBuffer &p_Buffer, VkDeviceMemory &p_BufferMemory );
		ErrorCode CopyBuffer( VkBuffer p_Source, VkBuffer p_Destination,
			VkDeviceSize p_Size );

		uint32_t GetSwapchainImageCount(
			VkSurfaceCapabilitiesKHR &p_SurfaceCapabilities );
		VkSurfaceFormatKHR GetSwapchainFormat(
			std::vector< VkSurfaceFormatKHR > &p_SurfaceFormats );
		VkExtent2D GetSwapchainExtent(
			VkSurfaceCapabilitiesKHR &p_SurfaceCapabilities );
		VkImageUsageFlags GetSwapchainUsageFlags(
			VkSurfaceCapabilitiesKHR &p_SurfaceCapabilities );
		VkSurfaceTransformFlagBitsKHR GetSwapchainTransformFlagBits(
			VkSurfaceCapabilitiesKHR &p_SurfaceCapabilities );
		VkPresentModeKHR GetSwapchainPresentMode(
			std::vector< VkPresentModeKHR > &p_PresentModes );

		uint32_t FindMemoryType( uint32_t p_TypeFilter,
			VkMemoryPropertyFlags p_Properties );

		Bool CheckExtensionAvailability( const char *p_pExtensionName,
			const std::vector< VkExtensionProperties > &p_Extensions );
		Bool CheckPhysicalDeviceProperties( VkPhysicalDevice p_PhysicalDevice,
			uint32_t &p_GraphicsQueueFamilyIndex,
			uint32_t &p_PresentQueueFamilyIndex );
		Bool CheckValidationLayerSupport(
			const std::vector< const char * > &p_ValidationLayers );

		//GameWindowData					*m_pGameWindowData;
		GameWindow						*m_pGameWindow;
		LibraryHandle					m_VulkanLibrary;
		VkInstance						m_VulkanInstance;
		VkDebugUtilsMessengerEXT		m_DebugMessenger;
		VkPhysicalDevice				m_VulkanPhysicalDevice;
		VkDevice						m_VulkanDevice;
		VkQueue							m_GraphicsQueue;
		uint32_t						m_GraphicsQueueFamilyIndex;
		VkQueue							m_PresentQueue;
		uint32_t						m_PresentQueueFamilyIndex;
		VkSurfaceKHR					m_PresentationSurface;
		std::vector< VkSemaphore >		m_ImageAvailableSemaphores;
		std::vector< VkSemaphore >		m_RenderingFinishedSemaphores;
		std::vector< VkFence >			m_InFlightFences;
		VkSwapchainKHR					m_Swapchain;
		VkExtent2D						m_SwapchainExtent;
		std::vector< VkImage >			m_SwapchainImages;
		std::vector< VkImageView >		m_SwapchainImageViews;
		VkFormat						m_SwapchainImageFormat;
		std::vector< VkFramebuffer >	m_SwapchainFramebuffers;
		VkPipelineLayout				m_PipelineLayout;
		VkPipeline						m_GraphicsPipeline;
		VkRenderPass					m_RenderPass;
		VkCommandPool					m_CommandPool;
		std::vector< VkCommandBuffer >	m_CommandBuffers;
		size_t							m_CurrentFrame;

		Bool				m_CanRender;
		Bool				m_Resize;

		// Example vertices
		std::vector< Vertex >			m_Vertices;
		VkBuffer						m_VertexBuffer;
		VkDeviceMemory					m_VertexBufferMemory;
	};
}

#endif // __BRAWL_RENDERER_HPP__

