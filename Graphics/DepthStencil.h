#pragma once

#include <Graphics/Types.h>

namespace Columbus
{

	struct DepthStencilOpDesc
	{
		StencilOp StencilFailOp;
		StencilOp StencilDepthFailOp;
		StencilOp StencilPassOp;
		ComparisonFunc StencilFunc;
	};

	struct DepthStencilStateDesc
	{
		bool DepthEnable;
		bool DepthWriteMask;
		ComparisonFunc DepthFunc;

		bool StencilEnable;
		uint8 StencilReadMask;
		uint8 StencilWriteMask;
		DepthStencilOpDesc FrontFace;
		DepthStencilOpDesc BackFace;
	};

	class DepthStencilState
	{
	public:
		DepthStencilStateDesc Desc;
	};

}
