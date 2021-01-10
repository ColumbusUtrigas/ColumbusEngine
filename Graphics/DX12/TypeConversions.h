#pragma once

#include <Graphics/InputLayout.h>
#include <Graphics/Types.h>
#include <d3d12.h>

namespace Columbus::Graphics::DX12
{

	static D3D12_PRIMITIVE_TOPOLOGY PrimitiveTopologyToDX12(PrimitiveTopology topology)
	{
		switch (topology)
		{
		case PrimitiveTopology::Undefined:    return D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
		case PrimitiveTopology::PointList:    return D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
		case PrimitiveTopology::LineList:     return D3D_PRIMITIVE_TOPOLOGY_LINELIST;
		case PrimitiveTopology::TriangleList: return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		case PrimitiveTopology::PatchList_1:  return D3D_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST;
		case PrimitiveTopology::PatchList_2:  return D3D_PRIMITIVE_TOPOLOGY_2_CONTROL_POINT_PATCHLIST;
		case PrimitiveTopology::PatchList_3:  return D3D_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST;
		case PrimitiveTopology::PatchList_4:  return D3D_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST;
		case PrimitiveTopology::PatchList_5:  return D3D_PRIMITIVE_TOPOLOGY_5_CONTROL_POINT_PATCHLIST;
		case PrimitiveTopology::PatchList_6:  return D3D_PRIMITIVE_TOPOLOGY_6_CONTROL_POINT_PATCHLIST;
		case PrimitiveTopology::PatchList_7:  return D3D_PRIMITIVE_TOPOLOGY_7_CONTROL_POINT_PATCHLIST;
		case PrimitiveTopology::PatchList_8:  return D3D_PRIMITIVE_TOPOLOGY_8_CONTROL_POINT_PATCHLIST;
		case PrimitiveTopology::PatchList_9:  return D3D_PRIMITIVE_TOPOLOGY_9_CONTROL_POINT_PATCHLIST;
		case PrimitiveTopology::PatchList_10: return D3D_PRIMITIVE_TOPOLOGY_10_CONTROL_POINT_PATCHLIST;
		case PrimitiveTopology::PatchList_11: return D3D_PRIMITIVE_TOPOLOGY_11_CONTROL_POINT_PATCHLIST;
		case PrimitiveTopology::PatchList_12: return D3D_PRIMITIVE_TOPOLOGY_12_CONTROL_POINT_PATCHLIST;
		case PrimitiveTopology::PatchList_13: return D3D_PRIMITIVE_TOPOLOGY_13_CONTROL_POINT_PATCHLIST;
		case PrimitiveTopology::PatchList_14: return D3D_PRIMITIVE_TOPOLOGY_14_CONTROL_POINT_PATCHLIST;
		case PrimitiveTopology::PatchList_15: return D3D_PRIMITIVE_TOPOLOGY_15_CONTROL_POINT_PATCHLIST;
		case PrimitiveTopology::PatchList_16: return D3D_PRIMITIVE_TOPOLOGY_16_CONTROL_POINT_PATCHLIST;
		case PrimitiveTopology::PatchList_17: return D3D_PRIMITIVE_TOPOLOGY_17_CONTROL_POINT_PATCHLIST;
		case PrimitiveTopology::PatchList_18: return D3D_PRIMITIVE_TOPOLOGY_18_CONTROL_POINT_PATCHLIST;
		case PrimitiveTopology::PatchList_19: return D3D_PRIMITIVE_TOPOLOGY_19_CONTROL_POINT_PATCHLIST;
		case PrimitiveTopology::PatchList_20: return D3D_PRIMITIVE_TOPOLOGY_20_CONTROL_POINT_PATCHLIST;
		case PrimitiveTopology::PatchList_21: return D3D_PRIMITIVE_TOPOLOGY_21_CONTROL_POINT_PATCHLIST;
		case PrimitiveTopology::PatchList_22: return D3D_PRIMITIVE_TOPOLOGY_22_CONTROL_POINT_PATCHLIST;
		case PrimitiveTopology::PatchList_23: return D3D_PRIMITIVE_TOPOLOGY_23_CONTROL_POINT_PATCHLIST;
		case PrimitiveTopology::PatchList_24: return D3D_PRIMITIVE_TOPOLOGY_24_CONTROL_POINT_PATCHLIST;
		case PrimitiveTopology::PatchList_25: return D3D_PRIMITIVE_TOPOLOGY_25_CONTROL_POINT_PATCHLIST;
		case PrimitiveTopology::PatchList_26: return D3D_PRIMITIVE_TOPOLOGY_26_CONTROL_POINT_PATCHLIST;
		case PrimitiveTopology::PatchList_27: return D3D_PRIMITIVE_TOPOLOGY_27_CONTROL_POINT_PATCHLIST;
		case PrimitiveTopology::PatchList_28: return D3D_PRIMITIVE_TOPOLOGY_28_CONTROL_POINT_PATCHLIST;
		case PrimitiveTopology::PatchList_29: return D3D_PRIMITIVE_TOPOLOGY_29_CONTROL_POINT_PATCHLIST;
		case PrimitiveTopology::PatchList_30: return D3D_PRIMITIVE_TOPOLOGY_30_CONTROL_POINT_PATCHLIST;
		case PrimitiveTopology::PatchList_31: return D3D_PRIMITIVE_TOPOLOGY_31_CONTROL_POINT_PATCHLIST;
		case PrimitiveTopology::PatchList_32: return D3D_PRIMITIVE_TOPOLOGY_32_CONTROL_POINT_PATCHLIST;
		}
	}

	static D3D12_PRIMITIVE_TOPOLOGY_TYPE PrimitiveTopologyTypeToDX12(PrimitiveTopology topology)
	{
		switch (topology)
		{
		case PrimitiveTopology::Undefined:    return D3D12_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED;
		case PrimitiveTopology::PointList:    return D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
		case PrimitiveTopology::LineList:     return D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
		case PrimitiveTopology::TriangleList: return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		case PrimitiveTopology::PatchList_1:
		case PrimitiveTopology::PatchList_2:
		case PrimitiveTopology::PatchList_3:
		case PrimitiveTopology::PatchList_4:
		case PrimitiveTopology::PatchList_5:
		case PrimitiveTopology::PatchList_6:
		case PrimitiveTopology::PatchList_7:
		case PrimitiveTopology::PatchList_8:
		case PrimitiveTopology::PatchList_9:
		case PrimitiveTopology::PatchList_10:
		case PrimitiveTopology::PatchList_11:
		case PrimitiveTopology::PatchList_12:
		case PrimitiveTopology::PatchList_13:
		case PrimitiveTopology::PatchList_14:
		case PrimitiveTopology::PatchList_15:
		case PrimitiveTopology::PatchList_16:
		case PrimitiveTopology::PatchList_17:
		case PrimitiveTopology::PatchList_18:
		case PrimitiveTopology::PatchList_19:
		case PrimitiveTopology::PatchList_20:
		case PrimitiveTopology::PatchList_21:
		case PrimitiveTopology::PatchList_22:
		case PrimitiveTopology::PatchList_23:
		case PrimitiveTopology::PatchList_24:
		case PrimitiveTopology::PatchList_25:
		case PrimitiveTopology::PatchList_26:
		case PrimitiveTopology::PatchList_27:
		case PrimitiveTopology::PatchList_28:
		case PrimitiveTopology::PatchList_29:
		case PrimitiveTopology::PatchList_30:
		case PrimitiveTopology::PatchList_31:
		case PrimitiveTopology::PatchList_32: return D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
		}
	}

	static DXGI_FORMAT IndexFormatToDX12(IndexFormat format)
	{
		switch (format)
		{
		case IndexFormat::Uint16: return DXGI_FORMAT_R16_UINT;
		case IndexFormat::Uint32: return DXGI_FORMAT_R32_UINT;
		}
	}

	static D3D12_FILL_MODE FillModeToDX12(FillMode fill)
	{
		switch (fill)
		{
		case FillMode::Solid:     return D3D12_FILL_MODE_SOLID;
		case FillMode::Wireframe: return D3D12_FILL_MODE_WIREFRAME;
		}
	}

	static D3D12_CULL_MODE CullModeToDX12(CullMode cull)
	{
		switch (cull)
		{
		case CullMode::No:    return D3D12_CULL_MODE_NONE;
		case CullMode::Front: return D3D12_CULL_MODE_FRONT;
		case CullMode::Back:  return D3D12_CULL_MODE_BACK;
		}
	}

	static D3D12_BLEND BlendToDX12(Blend blend)
	{
		switch (blend)
		{
		case Blend::Zero:           return D3D12_BLEND_ZERO;
		case Blend::One:            return D3D12_BLEND_ONE;
		case Blend::SrcColor:       return D3D12_BLEND_SRC_COLOR;
		case Blend::InvSrcColor:    return D3D12_BLEND_INV_SRC_COLOR;
		case Blend::SrcAlpha:       return D3D12_BLEND_SRC_ALPHA;
		case Blend::InvSrcAlpha:    return D3D12_BLEND_INV_SRC_ALPHA;
		case Blend::DestAlpha:      return D3D12_BLEND_DEST_ALPHA;
		case Blend::InvDestAlpha:   return D3D12_BLEND_INV_DEST_ALPHA;
		case Blend::DestColor:      return D3D12_BLEND_DEST_COLOR;
		case Blend::InvDestColor:   return D3D12_BLEND_INV_DEST_COLOR;
		case Blend::SrcAlphaSat:    return D3D12_BLEND_SRC_ALPHA_SAT;
		case Blend::BlendFactor:    return D3D12_BLEND_BLEND_FACTOR;
		case Blend::InvBlendFactor: return D3D12_BLEND_INV_BLEND_FACTOR;
		case Blend::Src1Color:      return D3D12_BLEND_SRC1_COLOR;
		case Blend::InvSrc1Color:   return D3D12_BLEND_INV_SRC1_COLOR;
		case Blend::Src1Alpha:      return D3D12_BLEND_SRC1_ALPHA;
		case Blend::InvSrc1Alpha:   return D3D12_BLEND_INV_SRC1_ALPHA;
		}
	}

	static D3D12_BLEND_OP BlendOpToDX12(BlendOp op)
	{
		switch (op)
		{
		case BlendOp::Add:         return D3D12_BLEND_OP_ADD;
		case BlendOp::Subtract:    return D3D12_BLEND_OP_SUBTRACT;
		case BlendOp::RevSubtract: return D3D12_BLEND_OP_REV_SUBTRACT;
		case BlendOp::Min:         return D3D12_BLEND_OP_MIN;
		case BlendOp::Max:         return D3D12_BLEND_OP_MAX;
		}
	}

	static D3D12_COMPARISON_FUNC ComparisonFuncToDX12(ComparisonFunc func)
	{
		switch (func)
		{
		case ComparisonFunc::Less:     return D3D12_COMPARISON_FUNC_LESS;
		case ComparisonFunc::Greater:  return D3D12_COMPARISON_FUNC_GREATER;
		case ComparisonFunc::LEqual:   return D3D12_COMPARISON_FUNC_LESS_EQUAL;
		case ComparisonFunc::GEqual:   return D3D12_COMPARISON_FUNC_GREATER_EQUAL;
		case ComparisonFunc::Equal:    return D3D12_COMPARISON_FUNC_EQUAL;
		case ComparisonFunc::NotEqual: return D3D12_COMPARISON_FUNC_NOT_EQUAL;
		case ComparisonFunc::Never:    return D3D12_COMPARISON_FUNC_NEVER;
		case ComparisonFunc::Always:   return D3D12_COMPARISON_FUNC_ALWAYS;
		}
	}

	static D3D12_STENCIL_OP StencilOpToDX12(StencilOp op)
	{
		switch (op)
		{
		case StencilOp::Keep:    return D3D12_STENCIL_OP_KEEP;
		case StencilOp::Zero:    return D3D12_STENCIL_OP_ZERO;
		case StencilOp::Replace: return D3D12_STENCIL_OP_REPLACE;
		case StencilOp::IncrSat: return D3D12_STENCIL_OP_INCR_SAT;
		case StencilOp::DecrSat: return D3D12_STENCIL_OP_DECR_SAT;
		case StencilOp::Invert:  return D3D12_STENCIL_OP_INVERT;
		case StencilOp::Incr:    return D3D12_STENCIL_OP_INCR;
		case StencilOp::Decr:    return D3D12_STENCIL_OP_DECR;
		}
	}

	static D3D12_INPUT_LAYOUT_DESC InputLayoutToDX12(const InputLayout& layout, D3D12_INPUT_ELEMENT_DESC elements[])
	{
		D3D12_INPUT_LAYOUT_DESC result {};

		for (size_t i = 0; i < layout.Elements.size(); i++)
		{
			DXGI_FORMAT format;
			switch (layout.Elements[i].Components)
			{
			case 2:
				format = DXGI_FORMAT_R32G32_FLOAT;
				break;
			case 4:
				format = DXGI_FORMAT_R32G32B32A32_FLOAT;
				break;
			}

			elements[i] = {
				layout.Elements[i].SemanticName,
				layout.Elements[i].SemanticIndex,
				format,
				layout.Elements[i].Slot,
				0,
				D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
				0
			};
		}

		result.NumElements = layout.Elements.size();
		result.pInputElementDescs = elements;

		return result;
	}

	static D3D12_RASTERIZER_DESC RasterizerStateToDX12(const RasterizerStateDesc& rsd)
	{
		D3D12_RASTERIZER_DESC result;

		result.FillMode = FillModeToDX12(rsd.Fill);
		result.CullMode = CullModeToDX12(rsd.Cull);
		result.FrontCounterClockwise = rsd.FrontCounterClockwise;
		result.DepthBias = rsd.DepthBias;
		result.DepthBiasClamp = rsd.DepthBiasClamp;
		result.SlopeScaledDepthBias = rsd.SlopeScaledDepthBias;
		result.DepthClipEnable = rsd.DepthClipEnable;
		result.MultisampleEnable = rsd.MultisampleEnable;
		result.AntialiasedLineEnable = rsd.AntialiasedLineEnable;
		result.ForcedSampleCount = 0;
		result.ConservativeRaster = rsd.ConservativeEnable ? D3D12_CONSERVATIVE_RASTERIZATION_MODE_ON : D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

		return result;
	}

	static D3D12_RENDER_TARGET_BLEND_DESC RenderTargetBlendToDX12(const RenderTargetBlendDesc& desc)
	{
		D3D12_RENDER_TARGET_BLEND_DESC result;

		result.BlendEnable = desc.BlendEnable;
		result.SrcBlend = BlendToDX12(desc.SrcBlend);
		result.DestBlend = BlendToDX12(desc.DestBlend);
		result.BlendOp = BlendOpToDX12(desc.Op);
		result.SrcBlendAlpha = BlendToDX12(desc.SrcBlendAlpha);
		result.DestBlendAlpha = BlendToDX12(desc.DestBlendAlpha);
		result.BlendOpAlpha = BlendOpToDX12(desc.OpAlpha);
		result.RenderTargetWriteMask = desc.RenderTargetWriteMask;
		result.LogicOpEnable = false;
		//result.LogicOp = ;

		return result;
	}

	static D3D12_BLEND_DESC BlendStateToDX12(const BlendStateDesc& bsd)
	{
		D3D12_BLEND_DESC result;

		result.AlphaToCoverageEnable = bsd.AlphaToCoverageEnable;
		result.IndependentBlendEnable = bsd.IndependentBlendEnable;

		for (size_t i = 0; i < bsd.NumRenderTargets; i++)
		{
			result.RenderTarget[i] = RenderTargetBlendToDX12(bsd.RenderTarget[i]);
		}

		return result;
	}

	static D3D12_DEPTH_STENCILOP_DESC DepthStencilOpDescToDX12(const DepthStencilOpDesc& desc)
	{
		D3D12_DEPTH_STENCILOP_DESC result;

		result.StencilFailOp = StencilOpToDX12(desc.StencilFailOp);
		result.StencilDepthFailOp = StencilOpToDX12(desc.StencilDepthFailOp);
		result.StencilPassOp = StencilOpToDX12(desc.StencilPassOp);
		result.StencilFunc = ComparisonFuncToDX12(desc.StencilFunc);

		return result;
	}

	static D3D12_DEPTH_STENCIL_DESC DepthStencilStateToDX12(const DepthStencilStateDesc& dssd)
	{
		D3D12_DEPTH_STENCIL_DESC result;

		result.DepthEnable = dssd.DepthEnable;
		result.DepthWriteMask = dssd.DepthWriteMask ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO;
		result.DepthFunc = ComparisonFuncToDX12(dssd.DepthFunc);
		result.StencilEnable = dssd.StencilEnable;
		result.StencilReadMask = dssd.StencilReadMask;
		result.StencilWriteMask = dssd.StencilWriteMask;
		result.FrontFace = DepthStencilOpDescToDX12(dssd.FrontFace);
		result.BackFace = DepthStencilOpDescToDX12(dssd.BackFace);

		return result;
	}

}
