#include "Graphics/Core/Types.h"
#include "Graphics/RenderGraph.h"
#include "RenderPasses.h"

namespace Columbus
{
	void DebugOverlayPass(RenderGraph& Graph, RenderGraphTextureRef Texture)
	{
		RenderPassParameters Parameters;
		Parameters.ColorAttachments[0] = RenderPassAttachment{ AttachmentLoadOp::Load, Texture, {} };
		RenderPassDependencies Dependencies;

		Graph.AddPass("DebugOverlay", RenderGraphPassType::Raster, Parameters, Dependencies, [](RenderGraphContext& Context)
		{
			
		});
	}
}
