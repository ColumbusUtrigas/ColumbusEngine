#pragma once

#include <Graphics/Types.h>

namespace Columbus
{

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

	class DepthStencilState
	{
	public:
		DepthStencilStateDesc Desc;
	public:
		const DepthStencilStateDesc& GetDesc() const { return Desc; }
	};

}
