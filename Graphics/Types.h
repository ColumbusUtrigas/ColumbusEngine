#pragma once

namespace Columbus
{

	#define RGBA_MASK(r, g, b, a) (r << 24| g << 16| b << 8 | a)

	enum class GraphicsAPI
	{
		None,
		OpenGL
	};

	struct SubresourceData
	{
		const void* pSysMem;
		size_t SysMemPitch;
		size_t SysMemSlicePitch;
	};

	enum class PrimitiveTopology
	{
		PointList,
		LineList,
		TriangleList
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
