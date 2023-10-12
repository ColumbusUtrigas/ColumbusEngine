#pragma once

#include "Common/Image/Image.h"
#include <Core/DataBlob.h>
#include <Core/Types.h>
#include <Core/fixed_vector.h>
#include <Math/Vector4.h>
#include <string>
#include <string_view>

namespace Columbus
{

	#define RGBA_MASK(r, g, b, a) (r << 24| g << 16| b << 8 | a)

	enum class GraphicsAPI
	{
		None,
		OpenGL,
		DX12,
		Vulkan,
		//Metal
	};

	struct SubresourceData
	{
		const void* pSysMem;
		size_t SysMemPitch;
		size_t SysMemSlicePitch;
	};

	struct Viewport
	{
		float TopLeftX;
		float TopLeftY;
		float Width;
		float Height;
		float MinDepth;
		float MaxDepth;

		Viewport() {}
		Viewport(float topLeftX, float topLeftY, float width, float height, float minDepth, float maxDepth) :
			TopLeftX(topLeftX), TopLeftY(topLeftY), Width(width), Height(height), MinDepth(minDepth), MaxDepth(maxDepth) {}
	};

	struct ScissorRect
	{
		uint32 Left;
		uint32 Top;
		uint32 Right;
		uint32 Bottom;

		ScissorRect() {}
		ScissorRect(uint32 left, uint32 top, uint32 right, uint32 bottom) :
			Left(left), Top(top), Right(right), Bottom(bottom) {}
	};

	enum class ShaderType : uint32_t
	{
		Vertex   = 1,
		Pixel    = 2,
		Hull     = 4,
		Domain   = 8,
		Geometry = 16,
		AllGraphics = Vertex | Pixel | Hull | Domain | Geometry,

		Compute  = 32,

		Raygen       = 64,
		Miss         = 128,
		Anyhit       = 256,
		ClosestHit   = 512,
		Intersection = 1024,
		AllRayTracing = Raygen | Miss | Anyhit | ClosestHit | Intersection,
	};

	inline constexpr bool operator!=(ShaderType x, uint64_t y) { return static_cast<uint64_t>(x) != y; }
	inline constexpr ShaderType operator|(ShaderType x, ShaderType y) { return static_cast<ShaderType>(static_cast<uint64_t>(x) | static_cast<uint64_t>(y)); }
	inline constexpr ShaderType operator&(ShaderType x, ShaderType y) { return static_cast<ShaderType>(static_cast<uint64_t>(x) & static_cast<uint64_t>(y)); }

	enum class ShaderLanguage
	{
		Undefined,
		CSL,
		HLSL,
		GLSL
	};

	// TODO: remove
	struct ShaderStage
	{
		std::string Source;
		std::string EntryPoint;
		ShaderType Type;
		ShaderLanguage Language;

		//ShaderStageBytecode Bytecode;

		ShaderStage() {}
		ShaderStage(std::string_view source, std::string_view entry, ShaderType type, ShaderLanguage language) :
			Source(source), EntryPoint(entry), Type(type), Language(language) {}
	};

	struct ShaderStageDesc
	{
		ShaderType Type;
		std::string EntryPoint;
	};

	enum class AttachmentType
	{
		Color,
		DepthStencil
	};

	enum class AttachmentLoadOp
	{
		Load,
		Clear,
		DontCare,
	};

	struct AttachmentClearValue
	{
		Vector4 Color;

		f32 Depth;
		u32 Stencil;

		bool operator==(const AttachmentClearValue&) const = default;
	};

	// LEGACY?
	struct AttachmentDesc
	{
		std::string Name;
		AttachmentType Type;
		AttachmentLoadOp LoadOp;
		TextureFormat Format;

		AttachmentDesc() {}
		AttachmentDesc(std::string Name, AttachmentType Type, AttachmentLoadOp LoadOp, TextureFormat Format) :
			Name(Name), Type(Type), LoadOp(LoadOp), Format(Format) {}
	};

	enum class PrimitiveTopology
	{
		Undefined,
		PointList,
		LineList,
		TriangleList,
		PatchList_1,
		PatchList_2,
		PatchList_3,
		PatchList_4,
		PatchList_5,
		PatchList_6,
		PatchList_7,
		PatchList_8,
		PatchList_9,
		PatchList_10,
		PatchList_11,
		PatchList_12,
		PatchList_13,
		PatchList_14,
		PatchList_15,
		PatchList_16,
		PatchList_17,
		PatchList_18,
		PatchList_19,
		PatchList_20,
		PatchList_21,
		PatchList_22,
		PatchList_23,
		PatchList_24,
		PatchList_25,
		PatchList_26,
		PatchList_27,
		PatchList_28,
		PatchList_29,
		PatchList_30,
		PatchList_31,
		PatchList_32
	};

	enum class IndexFormat
	{
		Uint16,
		Uint32
	};

	enum class ComparisonFunc
	{
		Less,
		Greater,
		LEqual,
		GEqual,
		Equal,
		NotEqual,
		Never,
		Always
	};

	enum class StencilOp
	{
		Keep,
		Zero,
		Replace,
		IncrSat,
		DecrSat,
		Invert,
		Incr,
		Decr
	};

	enum class Blend
	{
		Zero,
		One,
		SrcColor,
		InvSrcColor,
		SrcAlpha,
		InvSrcAlpha,
		DestAlpha,
		InvDestAlpha,
		DestColor,
		InvDestColor,
		SrcAlphaSat,
		BlendFactor,
		InvBlendFactor,
		Src1Color,
		InvSrc1Color,
		Src1Alpha,
		InvSrc1Alpha
	};

	enum class BlendOp
	{
		Add,
		Subtract,
		RevSubtract,
		Min,
		Max
	};

	enum class FillMode
	{
		Wireframe,
		Solid
	};

	enum class CullMode
	{
		No,
		Front,
		Back
	};

	enum COLOR_MASK
	{
		COLOR_MASK_RED = 1,
		COLOR_MASK_GREEN = 2,
		COLOR_MASK_BLUE = 4,
		COLOR_MASK_ALPHA = 8,
		COLOR_MASK_ALL = COLOR_MASK_RED | COLOR_MASK_GREEN | COLOR_MASK_BLUE | COLOR_MASK_ALPHA
	};

	struct RenderTargetBlendDesc
	{
		bool BlendEnable = false;
		Blend SrcBlend = Blend::One;
		Blend DestBlend = Blend::Zero;
		BlendOp Op = BlendOp::Add;
		Blend SrcBlendAlpha = Blend::One;
		Blend DestBlendAlpha = Blend::Zero;
		BlendOp OpAlpha = BlendOp::Add;
		uint8 RenderTargetWriteMask = COLOR_MASK_ALL;
	};

	struct BlendStateDesc
	{
		bool AlphaToCoverageEnable = false;
		bool IndependentBlendEnable = false;
		fixed_vector<RenderTargetBlendDesc, 8> RenderTargets;
	};

	struct RasterizerStateDesc
	{
		FillMode Fill = FillMode::Solid;
		CullMode Cull = CullMode::Back;
		bool FrontCounterClockwise = false;
		int DepthBias = 0;
		float DepthBiasClamp = 0;
		float SlopeScaledDepthBias = 0;
		bool DepthClipEnable = true;
		bool ScissorEnable = false;
		bool MultisampleEnable = false;
		bool AntialiasedLineEnable = false;
		bool ConservativeEnable = false;
	};

	struct DepthStencilOpDesc
	{
		StencilOp StencilFailOp = StencilOp::Keep;
		StencilOp StencilDepthFailOp = StencilOp::Keep;
		StencilOp StencilPassOp = StencilOp::Keep;
		ComparisonFunc StencilFunc = ComparisonFunc::Always;
	};

	struct DepthStencilStateDesc
	{
		bool DepthEnable = true;
		bool DepthWriteMask = true;
		ComparisonFunc DepthFunc = ComparisonFunc::Less;

		bool StencilEnable = false;
		uint8 StencilReadMask = 0xFF;
		uint8 StencilWriteMask = 0xFF;
		DepthStencilOpDesc FrontFace;
		DepthStencilOpDesc BackFace;
	};
}
