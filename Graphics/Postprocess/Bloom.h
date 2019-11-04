#pragma once

#include <Graphics/ScreenQuad.h>
#include <Graphics/PostEffect.h>

#include <array>

namespace Columbus
{

	enum class BloomResolutionType
	{
		Quad = 0,
		Half,
		Full
	};

	class PostprocessBloom
	{
	private:
		ScreenQuad& _Quad;
	public:
		bool Enabled = false;
		float Treshold = 3.0f;
		float Intensity = 0.1f;
		float Radius = 1.0f;
		int Iterations = 2;
		BloomResolutionType Resolution = BloomResolutionType::Quad;
	public:
		PostprocessBloom(ScreenQuad& Quad) :
			_Quad(Quad) {}
		~PostprocessBloom() = default;

		void Draw(Texture*& Frame, const iVector2& FrameSize,
			std::array<PostEffect, 2>& Post);
	};

}


