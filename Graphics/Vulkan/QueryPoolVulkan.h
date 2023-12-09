#pragma once

#include <Graphics/Core/GraphicsCore.h>
#include <vulkan/vulkan.h>

namespace Columbus
{

	class QueryPoolVulkan : public QueryPool
	{
	public:
		VkQueryPool _Pool;
	public:
		QueryPoolVulkan(const QueryPoolDesc& Desc) : QueryPool(Desc) {}
	};

}
