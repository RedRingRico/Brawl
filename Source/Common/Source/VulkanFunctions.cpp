#include <vulkan/vulkan.h>
#include <DataTypes.hpp>

namespace Brawl
{
#define VK_EXPORTED_FUNCTION( p_Function ) \
	PFN_##p_Function p_Function = Null;

#define VK_GLOBAL_LEVEL_FUNCTION( p_Function ) \
	PFN_##p_Function p_Function = Null;

#define VK_INSTANCE_LEVEL_FUNCTION( p_Function ) \
	PFN_##p_Function p_Function = Null;

#define VK_DEVICE_LEVEL_FUNCTION( p_Function ) \
	PFN_##p_Function p_Function = Null;

#include <VulkanFunctions.inl>
}

