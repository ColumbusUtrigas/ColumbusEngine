#pragma once

#include <Graphics/DepthStencilState.h>
#include <Graphics/RasterizerState.h>
#include <Graphics/BlendState.h>

void GL_set_depth_stencil_state(const Columbus::DepthStencilStateDesc& state);
void GL_set_rasterizer_state(const Columbus::RasterizerStateDesc& state);
void GL_set_blend_state(const Columbus::BlendStateDesc& state);
