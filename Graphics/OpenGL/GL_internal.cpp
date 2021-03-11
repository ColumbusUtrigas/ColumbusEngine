#include "GL_internal.h"
#include <Graphics/OpenGL/TypeConversions.h>
#include <GL/glew.h>

void GL_set_depth_stencil_state(const Columbus::DepthStencilStateDesc& state)
{
	if (state.DepthEnable)
		glEnable(GL_DEPTH_TEST);
	else
		glDisable(GL_DEPTH_TEST);

	glDepthMask(state.DepthWriteMask ? GL_TRUE : GL_FALSE);
	glDepthFunc(ComparisonFuncToGL(state.DepthFunc));
}

void GL_set_rasterizer_state(const Columbus::RasterizerStateDesc& state)
{
	switch (state.Cull)
	{
	case Columbus::CullMode::No:
		glDisable(GL_CULL_FACE);
		break;
	case Columbus::CullMode::Front:
		glEnable(GL_CULL_FACE);
		glCullFace(GL_FRONT);
		glFrontFace(state.FrontCounterClockwise ? GL_CCW : GL_CW);
		break;
	case Columbus::CullMode::Back:
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		glFrontFace(state.FrontCounterClockwise ? GL_CCW : GL_CW);
		break;
	}

	switch (state.Fill)
	{
	case Columbus::FillMode::Wireframe:
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		break;
	case Columbus::FillMode::Solid:
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		break;
	}
}

void GL_set_blend_state(const Columbus::BlendStateDesc& state)
{
	if (state.AlphaToCoverageEnable)
	{
		glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE);
	}
	else
	{
		glDisable(GL_SAMPLE_ALPHA_TO_COVERAGE);
	}

	bool bBlendEnable = false;
	for (int i = 0; i < state.NumRenderTargets; i++)
	{
		if (state.RenderTarget[i].BlendEnable)
			bBlendEnable = true;
	}

	if (bBlendEnable)
		glEnable(GL_BLEND);
	else
		glDisable(GL_BLEND);

	if (state.IndependentBlendEnable)
	{
		for (int i = 0; i < state.NumRenderTargets; i++)
		{
			const auto& RTi = state.RenderTarget[i];
			if (RTi.BlendEnable)
			{
				glEnablei(GL_BLEND, i);
				glBlendFuncSeparatei(i, BlendToGL(RTi.SrcBlend), BlendToGL(RTi.DestBlend), BlendToGL(RTi.SrcBlendAlpha), BlendToGL(RTi.DestBlendAlpha));
				glBlendEquationSeparatei(i, BlendOpToGL(RTi.Op), BlendOpToGL(RTi.OpAlpha));
			}
			else
			{
				glDisablei(GL_BLEND, i);
			}
		}
	}
	else
	{
		const auto& RT0 = state.RenderTarget[0];
		if (RT0.BlendEnable)
		{
			glBlendFuncSeparate(BlendToGL(RT0.SrcBlend), BlendToGL(RT0.DestBlend), BlendToGL(RT0.SrcBlendAlpha), BlendToGL(RT0.DestBlendAlpha));
			glBlendEquationSeparate(BlendOpToGL(RT0.Op), BlendOpToGL(RT0.OpAlpha));
		}
		else
		{
			glDisable(GL_BLEND);
		}
	}
}
