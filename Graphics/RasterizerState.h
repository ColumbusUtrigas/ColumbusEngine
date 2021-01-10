#pragma once

#include <Graphics/Types.h>

namespace Columbus
{

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

    class RasterizerState
    {
    public:
        RasterizerStateDesc Desc;
    public:
        const RasterizerStateDesc& GetDesc() const { return Desc; }
    };

}
