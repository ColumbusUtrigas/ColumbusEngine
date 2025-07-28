#include "RenderPasses.h"

// vulkan
#include "Graphics/Vulkan/TypeConversions.h"

// third party
#include <Lib/FidelityFX-SDK/sdk/include/FidelityFX/host/backends/vk/ffx_vk.h>
#include <Lib/FidelityFX-SDK/sdk/include/FidelityFX/host/ffx_blur.h>
#include <Lib/FidelityFX-SDK/sdk/include/FidelityFX/host/ffx_dof.h>

namespace Columbus
{

	// Global profiling counters
	DECLARE_GPU_PROFILING_COUNTER(GpuCounterDoF);

	IMPLEMENT_GPU_PROFILING_COUNTER("DoF", "RenderGraphGPU", GpuCounterDoF);

	// Internal structs
	struct InternalFfxDofState
	{
		FfxDofContext* DofContext;

		iVector2 Size;
		u32      Quality;
		float    CocLimitFactor;
	};

	// Internal functions
	static FfxInterface* GetFfxInterface(FidelityFXContext* pContext)
	{
		return (FfxInterface*)pContext->InterfaceFFX;
	}

	static FfxResource GetFfxResourceFromTexture(Texture2* Texture, FfxResourceUsage Usage, FfxResourceStates State = FFX_RESOURCE_STATE_COMMON)
	{
		TextureVulkan* texvk = static_cast<TextureVulkan*>(Texture);
		TextureDesc2 Desc = Texture->GetDesc();

		FfxResourceDescription ResourceDesc;
		ResourceDesc.type     = FFX_RESOURCE_TYPE_TEXTURE2D;
		ResourceDesc.format   = ffxGetSurfaceFormatVK(TextureFormatToVK(Desc.Format));
		ResourceDesc.width    = Desc.Width;
		ResourceDesc.height   = Desc.Height;
		ResourceDesc.depth    = Desc.Depth;
		ResourceDesc.mipCount = Desc.Mips; // Number of mips (or 0 for full mipchain).
		ResourceDesc.flags    = FFX_RESOURCE_FLAGS_NONE;
		ResourceDesc.usage    = Usage;

		return ffxGetResourceVK(texvk->_Image, ResourceDesc, nullptr, State);
	}

	static void InitFfxBlur(FidelityFXContext* pContext)
	{
		u32 KernelSizes =
			FFX_BLUR_KERNEL_SIZE_3x3 |
			FFX_BLUR_KERNEL_SIZE_5x5 |
			FFX_BLUR_KERNEL_SIZE_7x7 |
			FFX_BLUR_KERNEL_SIZE_9x9 |
			FFX_BLUR_KERNEL_SIZE_11x11 |
			FFX_BLUR_KERNEL_SIZE_13x13 |
			FFX_BLUR_KERNEL_SIZE_15x15 |
			FFX_BLUR_KERNEL_SIZE_17x17 |
			FFX_BLUR_KERNEL_SIZE_19x19 |
			FFX_BLUR_KERNEL_SIZE_21x21;

		pContext->BlurContext = new FfxBlurContext();

		FfxBlurContext* BlurContext = (FfxBlurContext*)pContext->BlurContext;
		FfxBlurContextDescription BlurContextDesc;
		BlurContextDesc.kernelPermutations = FFX_BLUR_KERNEL_PERMUTATION_0 | FFX_BLUR_KERNEL_PERMUTATION_1 | FFX_BLUR_KERNEL_PERMUTATION_2;
		BlurContextDesc.kernelSizes = KernelSizes;
		BlurContextDesc.floatPrecision = FFX_BLUR_FLOAT_PRECISION_32BIT;
		BlurContextDesc.backendInterface = *GetFfxInterface(pContext);

		FfxErrorCode Err = ffxBlurContextCreate(BlurContext, &BlurContextDesc);
		if (Err != FFX_OK)
		{
			Log::Error("FFX Blur failed to initialise");
			DEBUGBREAK();
		}

		Log::Initialization("FFX Blur initialised");
	}

	static void DestroyFfxBlur(FidelityFXContext* pContext)
	{
		ffxBlurContextDestroy((FfxBlurContext*)pContext->BlurContext);
		delete pContext->BlurContext;
		pContext->BlurContext = nullptr;
	}

	static void UpdateFfxDof(SPtr<DeviceVulkan> Device, FidelityFXContext* pContext, iVector2 Size, u32 Quality, float CocLimitFactor)
	{
		if (pContext->DofState == nullptr)
		{
			pContext->DofState = new InternalFfxDofState();
			memset(pContext->DofState, 0, sizeof(InternalFfxDofState));
		}
		InternalFfxDofState* DofState = (InternalFfxDofState*)pContext->DofState;

		// when any state changes - recreate context
		if (DofState->DofContext == nullptr || DofState->Size != Size || DofState->Quality != Quality || DofState->CocLimitFactor != CocLimitFactor)
		{
			if (DofState->DofContext)
			{
				Device->QueueWaitIdle();
				ffxDofContextDestroy(DofState->DofContext);
				delete DofState->DofContext;
			}

			DofState->DofContext = new FfxDofContext();
			DofState->Size = Size;
			DofState->Quality = Quality;
			DofState->CocLimitFactor = CocLimitFactor;

			FfxDofContextDescription ContextDesc;
			ContextDesc.flags = FFX_DOF_DISABLE_RING_MERGE | FFX_DOF_OUTPUT_PRE_INIT;
			ContextDesc.quality = Quality;
			ContextDesc.resolution.width = Size.X;
			ContextDesc.resolution.height = Size.Y;
			ContextDesc.backendInterface = *GetFfxInterface(pContext);
			ContextDesc.cocLimitFactor = CocLimitFactor;

			FfxErrorCode Err = ffxDofContextCreate(DofState->DofContext, &ContextDesc);
			if (Err != FFX_OK)
			{
				Log::Error("FFX DoF context creation failed");
				DEBUGBREAK();
			}
		}
	}

	static void DestroyFfxDof(FidelityFXContext* pContext)
	{
		InternalFfxDofState* DofState = (InternalFfxDofState*)pContext->DofState;

		if (DofState)
		{
			ffxDofContextDestroy(DofState->DofContext);

			delete DofState;
			pContext->DofState = nullptr;
		}
	}

	void InitDeferredRenderContext(SPtr<DeviceVulkan> Device, DeferredRenderContext* pContext)
	{
		if (pContext->FFX)
		{
			Log::Error("FFX was already initialised");
			DEBUGBREAK();
		}

		pContext->FFX = new FidelityFXContext();

		FfxInterface* InterfaceFfx = new FfxInterface();
		pContext->FFX->InterfaceFFX = (void*)InterfaceFfx;

		VkDeviceContext DeviceContext;
		DeviceContext.vkDevice = Device->_Device;
		DeviceContext.vkPhysicalDevice = Device->_PhysicalDevice;
		DeviceContext.vkDeviceProcAddr = vkGetDeviceProcAddr;

		FfxDevice DeviceFfx = ffxGetDeviceVK(&DeviceContext);

		const int MaxContexts = 4;
		size_t ScratchBufferSize = ffxGetScratchMemorySizeVK(Device->_PhysicalDevice, MaxContexts);
		u8* ScratchBuffer = new u8[ScratchBufferSize];
		memset(ScratchBuffer, 0, ScratchBufferSize);

		FfxErrorCode Err = ffxGetInterfaceVK(InterfaceFfx, DeviceFfx, ScratchBuffer, ScratchBufferSize, MaxContexts);
		if (Err != FFX_OK)
		{
			Log::Error("FFX failed to initialise");
			DEBUGBREAK();
		}

		Log::Initialization("FFX initialised");

		InitFfxBlur(pContext->FFX);
	}

	void ShutdownDeferredRenderContext(SPtr<DeviceVulkan> Device, DeferredRenderContext* pContext)
	{
		pContext->History.Destroy(Device);

		DestroyFfxBlur(pContext->FFX);
		DestroyFfxDof(pContext->FFX);

		delete pContext->FFX->InterfaceFFX;
		delete pContext->FFX;
		pContext->FFX = nullptr;
	}

	// implementation of public functions
namespace FFX
{


	void DispatchGaussianBlur(CommandBufferVulkan* CmdBuf, Texture2* Source, Texture2* Target, FidelityFXContext* Context, EFFXBlurSigma Sigma, EFFXBlurKernelSize KernelSize)
	{
		FfxBlurDispatchDescription DispatchDesc;
		DispatchDesc.commandList = ffxGetCommandListVK(CmdBuf->_CmdBuf);
		DispatchDesc.kernelPermutation = (FfxBlurKernelPermutation)Sigma;
		DispatchDesc.kernelSize = (FfxBlurKernelSize)KernelSize;
		DispatchDesc.inputAndOutputSize.width  = Source->GetDesc().Width;
		DispatchDesc.inputAndOutputSize.height = Source->GetDesc().Height;
		DispatchDesc.input  = GetFfxResourceFromTexture(Source, FFX_RESOURCE_USAGE_UAV);
		DispatchDesc.output = GetFfxResourceFromTexture(Target, FFX_RESOURCE_USAGE_UAV);

		FfxErrorCode Err = ffxBlurContextDispatch((FfxBlurContext*)Context->BlurContext, &DispatchDesc);
		if (Err != FFX_OK)
		{
			Log::Error("FFX Blur dispatch failed");
			DEBUGBREAK();
		}
	}

	RenderGraphTextureRef DispatchGaussianBlurRG(RenderGraph& Graph, RenderGraphTextureRef Source, FidelityFXContext* FXContext, EFFXBlurSigma Sigma, EFFXBlurKernelSize KernelSize)
	{
		RenderPassParameters Parameters;

		//char NameBuf[512];
		//snprintf(NameBuf, 512, "%s (FFX Blur Result)", )
		RenderGraphTextureRef Target = Graph.CreateTexture(Graph.GetTextureDesc(Source), "FFX Blur Result");

		RenderPassDependencies Dependencies(Graph.Allocator);
		Dependencies.Read(Source, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
		Dependencies.Write(Target, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

		Graph.AddPass("FFX Blur", RenderGraphPassType::Compute, Parameters, Dependencies, [Target, Sigma, KernelSize, FXContext, Source](RenderGraphContext& Context)
		{
			Texture2* SourceTex = Context.GetRenderGraphTexture(Source).get();
			Texture2* TargetTex = Context.GetRenderGraphTexture(Target).get();
			DispatchGaussianBlur(Context.CommandBuffer, SourceTex, TargetTex, FXContext, Sigma, KernelSize);
		});

		return Target;
	}

	RenderGraphTextureRef DispatchDepthOfFieldRG(RenderGraph& Graph, RenderView& View, SceneTextures& Textures, FidelityFXContext* FXContext)
	{
		RenderGraphTextureRef Source = Textures.FinalBeforeTonemap;
		RenderGraphTextureRef Depth = Textures.GBufferDS;

		if (!View.EffectsSettings.DepthOfField.EnableDoF)
		{
			return Source;
		}

		iVector2 Size = Graph.GetTextureSize2D(Source);

		UpdateFfxDof(Graph.Device, FXContext, Size, 10, 0.1f);

		FfxDofContext* DofContext = ((InternalFfxDofState*)FXContext->DofState)->DofContext;

		RenderPassParameters Parameters;

		RenderPassDependencies Dependencies(Graph.Allocator);
		Dependencies.Write(Source, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
		Dependencies.Read(Depth, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

		Graph.AddPass("FFX DoF", RenderGraphPassType::Compute, Parameters, Dependencies, [View, DofContext, Source, Depth](RenderGraphContext& Context)
		{
			RENDER_GRAPH_PROFILE_GPU_SCOPED(GpuCounterDoF, Context);

			Texture2* SourceTex = Context.GetRenderGraphTexture(Source).get();
			Texture2* DepthTex  = Context.GetRenderGraphTexture(Depth).get();
			Texture2* TargetTex = SourceTex;

			const HDepthOfFieldSettings& DofSettings = View.EffectsSettings.DepthOfField;

			const float FocusDist   = DofSettings.FocusDistance;
			const float SensorSize  = DofSettings.SensorSize;
			const float FocalLength = View.CameraCur.GetFocalLength(DofSettings.SensorSize);
			const float Aperture    = View.CameraCur.GetApertureDiameter(DofSettings.SensorSize, DofSettings.FStop);

			Matrix Proj = View.CameraCur.GetProjectionMatrix();

			float Conversion = 0.5f * (float)SourceTex->GetDesc().Width / SensorSize;
			float Proj33 = Proj.M[2][2];
			float Proj34 = Proj.M[2][3];
			float Proj43 = Proj.M[3][2];

			Context.CommandBuffer->TransitionImageLayout(SourceTex, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
			Context.CommandBuffer->TransitionImageLayout(DepthTex, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

			FfxDofDispatchDescription DispatchDesc;
			DispatchDesc.commandList = ffxGetCommandListVK(Context.CommandBuffer->_CmdBuf);
			DispatchDesc.color    = GetFfxResourceFromTexture(SourceTex, FFX_RESOURCE_USAGE_UAV, FFX_RESOURCE_STATE_PIXEL_COMPUTE_READ);
			DispatchDesc.depth    = GetFfxResourceFromTexture(DepthTex,  FFX_RESOURCE_USAGE_DEPTHTARGET, FFX_RESOURCE_STATE_PIXEL_COMPUTE_READ);
			DispatchDesc.output   = GetFfxResourceFromTexture(TargetTex, FFX_RESOURCE_USAGE_UAV, FFX_RESOURCE_STATE_PIXEL_COMPUTE_READ);
			DispatchDesc.cocScale = ffxDofCalculateCocScale(Aperture, -FocusDist, FocalLength, Conversion, Proj33, Proj34, Proj43);
			DispatchDesc.cocBias  = ffxDofCalculateCocBias(Aperture, -FocusDist, FocalLength, Conversion, Proj33, Proj34, Proj43);

			FfxErrorCode Err = ffxDofContextDispatch(DofContext, &DispatchDesc);
			if (Err != FFX_OK)
			{
				Log::Error("FFX DoF dispatch failed");
				DEBUGBREAK();
			}
		});

		return Source;
	}

	RenderGraphTextureRef DispatchReflectionsDenoiserRG(RenderGraph& Graph, RenderView& View, SceneTextures& Textures, FidelityFXContext* Context)
	{
		// TODO: update context
		// TODO: get all resources
		// TODO: dispatch

		return Textures.RTReflections;
	}
} // namespace FFX

}
