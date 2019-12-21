#pragma once

#include <Core/Types.h>
#include <Graphics/Types.h>

namespace Columbus
{

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
		RenderTargetBlendDesc RenderTarget[8];
	};

	class BlendState
	{
	public:
		BlendStateDesc Desc;
	public:
		const BlendStateDesc& GetDesc() const { return Desc; }
	};

}
