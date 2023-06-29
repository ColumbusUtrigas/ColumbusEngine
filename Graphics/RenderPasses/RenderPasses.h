#pragma once

#include "Common/Image/Image.h"
#include "Graphics/Core/GraphicsCore.h"
#include "Graphics/IrradianceVolume.h"
#include <Graphics/RenderGraph.h>
#include <Graphics/Camera.h>

namespace Columbus
{

	// **********************************
	// Real-time
	//

	class GBufferPass : public RenderPass
	{
	public:
		static constexpr const char* RTAlbedo = "GBufferAlbedo";
		static constexpr const char* RTNormal = "GBufferNormal";
		static constexpr const char* RTWorldPosition = "GBufferWorldPosition";
		static constexpr const char* RTRoughnessMetallness = "GBufferRoughnessMetallness";
		static constexpr const char* RTDepth = "GBufferDepth";
	public:
		GBufferPass(Camera& Camera) : MainCamera(Camera), RenderPass("GBufferPass")
		{
			IsGraphicsPass = true;

			AddOutputRenderTarget(AttachmentDesc(RTAlbedo, AttachmentType::Color, AttachmentLoadOp::Clear, TextureFormat::RGBA8));
			AddOutputRenderTarget(AttachmentDesc(RTNormal, AttachmentType::Color, AttachmentLoadOp::Clear, TextureFormat::RGBA16F));
			AddOutputRenderTarget(AttachmentDesc(RTWorldPosition, AttachmentType::Color, AttachmentLoadOp::Clear, TextureFormat::RGBA32F));
			AddOutputRenderTarget(AttachmentDesc(RTRoughnessMetallness, AttachmentType::Color, AttachmentLoadOp::Clear, TextureFormat::RG16F));
			AddOutputRenderTarget(AttachmentDesc(RTDepth, AttachmentType::DepthStencil, AttachmentLoadOp::Clear, TextureFormat::Depth24));
		}

		virtual void Setup(RenderGraphContext& Context) override;
		virtual void PreExecute(RenderGraphContext& Context) override;
		virtual void Execute(RenderGraphContext& Context) override;

	private:
		GraphicsPipeline* Pipeline;
		Camera& MainCamera;
	};

	class GBufferCompositePass : public RenderPass
	{
	public:
		GBufferCompositePass() : RenderPass("GBufferCompositePass")
		{
			IsGraphicsPass = true;

			AddOutputRenderTarget(AttachmentDesc(FinalColorOutput, AttachmentType::Color, AttachmentLoadOp::Clear, TextureFormat::BGRA8SRGB));
		}

		virtual void Setup(RenderGraphContext& Context) override;
		virtual void PreExecute(RenderGraphContext& Context) override;
		virtual void Execute(RenderGraphContext& Context) override;
	private:
		GraphicsPipeline* Pipeline;

		Texture2* InputAlbedo;
		Texture2* InputNormal;
		Texture2* InputShadowBuffer;
	};

	class RayTracedShadowsPass : public RenderPass
	{
	public:
		static constexpr const char* RTShadows = "RayTracedShadowsBuffer";
	public:
		RayTracedShadowsPass() : RenderPass("RayTracedShadows")
		{
		}

		virtual void Setup(RenderGraphContext& Context) override;
		virtual void PreExecute(RenderGraphContext& Context) override;
		virtual void Execute(RenderGraphContext& Context) override;
	private:
		RayTracingPipeline* Pipeline;

		Texture2* InputNormal;
		Texture2* InputDepth;
		Texture2* InputWorldPosition;
	};

	class ForwardShadingPass : public RenderPass
	{
	public:
		ForwardShadingPass(Camera& Camera, IrradianceVolume& Volume) : Volume(Volume), MainCamera(Camera), RenderPass("Forward Shading Pass")
		{
			IsGraphicsPass = true;

			AddOutputRenderTarget(AttachmentDesc(FinalColorOutput, AttachmentType::Color, AttachmentLoadOp::Clear, TextureFormat::BGRA8SRGB));
			AddOutputRenderTarget(AttachmentDesc("Depth", AttachmentType::DepthStencil, AttachmentLoadOp::Clear, TextureFormat::Depth24));
		}

		virtual void Setup(RenderGraphContext& Context) override;
		virtual void PreExecute(RenderGraphContext& Context) override;
		virtual void Execute(RenderGraphContext& Context) override;
	private:
		GraphicsPipeline* Pipeline;
		GraphicsPipeline* ProbeVisPipeline;
		Camera& MainCamera;
		IrradianceVolume& Volume;
	};

	class IrradianceProbeTracePass  : public RenderPass
	{
	public:
		static constexpr const char* ProbeBufferName = "IrradianceProbeBuffer";
	public:
		IrradianceProbeTracePass(IrradianceVolume& InVolume) : Volume(InVolume), RenderPass("Irradiance Probe Trace Pass")
		{

		}

		virtual void Setup(RenderGraphContext& Context) override;
		virtual void PreExecute(RenderGraphContext& Context) override;
		virtual void Execute(RenderGraphContext& Context) override;
	private:
		RayTracingPipeline* Pipeline;
		IrradianceVolume& Volume;
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
			AddOutputRenderTarget(AttachmentDesc(FinalColorOutput, AttachmentType::Color, AttachmentLoadOp::Clear, TextureFormat::BGRA8SRGB));
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
