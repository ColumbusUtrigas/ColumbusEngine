#pragma once

#include <Core/DataBlob.h>
#include <Core/Types.h>
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
		//Vulkan,
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

	struct ShaderStageBytecode
	{
		DataBlob Instruction;
	};

	enum class ShaderType
	{
		Vertex,
		Pixel,
		Hull,
		Domain,
		Geometry,
		Compute
	};

	enum class ShaderLanguage
	{
		Undefined,
		CSL,
		HLSL,
		GLSL
	};

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
}
