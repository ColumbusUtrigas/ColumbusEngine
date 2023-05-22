#pragma once

#include <Graphics/RenderGraph.h>
#include <Graphics/Camera.h>

namespace Columbus
{

	class VisBufferPass : public RenderPass
	{

	};

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
