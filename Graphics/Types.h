#pragma once

namespace Columbus
{

	enum class PrimitiveTopology
	{
		TriangleList
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

}
