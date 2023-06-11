#pragma once

#include "Core/fixed_vector.h"
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
		// static constexpr int NumRenderTargets = 8;
		fixed_vector<RenderTargetBlendDesc, 8> RenderTargets;

		// int UsedRenderTargets = 0;
		// RenderTargetBlendDesc RenderTarget[NumRenderTargets];
	};

	class BlendState
	{
	public:
		BlendStateDesc Desc;
	public:
		const BlendStateDesc& GetDesc() const { return Desc; }
	};

}
