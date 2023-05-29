#pragma once

#include "Common/Image/Image.h"
#include "Graphics/GraphicsPipeline.h"
#include "Graphics/Types.h"
#include <Graphics/RenderGraph.h>
#include <Graphics/Camera.h>

namespace Columbus
{

	// **********************************
	// Real-time
	//

	class ForwardShadingPass : public RenderPass
	{
	public:
		ForwardShadingPass(Camera& Camera) : MainCamera(Camera), RenderPass("Forward Shading Pass")
		{
			IsGraphicsPass = true;

			AddOutputRenderTarget(AttachmentDesc(FinalColorOutput, AttachmentType::Color, AttachmentLoadOp::Clear, TextureFormat::BGRA8SRGB));
			AddOutputRenderTarget(AttachmentDesc("Depth", AttachmentType::DepthStencil, AttachmentLoadOp::Load, TextureFormat::Depth24));
		}

		virtual void Setup(RenderGraphContext& Context) override;
		virtual void PreExecute(RenderGraphContext& Context) override;
		virtual void Execute(RenderGraphContext& Context) override;
	private:
		GraphicsPipeline* Pipeline;
		Camera& MainCamera;
	};

	// **********************************
	// Path-Tracing
	//

	class PathTracePass : public RenderPass
	{
	public:
		static constexpr const char* RenderTargetName = "PathTrace Frame result";
	public:
		PathTracePass(Camera& Camera) : MainCamera(Camera), RenderPass("Path Tracing Pass") {}

		virtual void Setup(RenderGraphContext& Context) override;
		virtual void Execute(RenderGraphContext& Context) override;
	private:
		RayTracingPipeline* PTPipeline;
		Camera& MainCamera;
	};

	class PathTraceDisplayPass : public RenderPass
	{
	public:
		PathTraceDisplayPass() : RenderPass("Path Tracing Display Pass")
		{
			IsGraphicsPass = true;
		}

		virtual void Setup(RenderGraphContext& Context) override;
		virtual void PreExecute(RenderGraphContext& Context) override;
		virtual void Execute(RenderGraphContext& Context) override;
	private:
		GraphicsPipeline* Pipeline;
		Texture2* TraceResult;
		uint Frame = 1;
	};

}
