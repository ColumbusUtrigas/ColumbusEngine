#pragma once

#include "Graphics/Core/GraphicsCore.h"
#include "Graphics/RenderGraph.h"

namespace Columbus::RadianceCache
{

	struct RadianceCacheData
	{
		RenderGraphBufferId DataBuffer;
		//Buffer* DataBuffer = nullptr;
	};

	void TraceRadianceCache(RenderGraph& Graph, RenderView& View, RadianceCacheData& RadianceCache);
	RenderGraphTextureRef VisualiseRadianceCache(RenderGraph& Graph, RenderView& View, RadianceCacheData& RadianceCache, RenderGraphTextureRef GBufferWP);

}
