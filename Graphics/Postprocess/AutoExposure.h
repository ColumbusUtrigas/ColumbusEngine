#pragma once

#include <Graphics/OpenGL/BufferOpenGL.h>
#include <Graphics/ScreenQuad.h>
#include <Graphics/PostEffect.h>
#include <Graphics/Device.h>

namespace Columbus
{

	class PostprocessAutoExposure
	{
	private:
		ScreenQuad& _Quad;
		bool _FirstDraw = true;

		static constexpr int _BufSize = 2;
		PostEffect _Bufs[_BufSize];
		int _CurrentBuf = 0;
	public:
		bool Enabled = false;
		float Min = 0.1f;
		float Max = 1.0f;
		float SpeedUp = 1.0f;
		float SpeedDown = 1.0f;
	public:
		PostprocessAutoExposure(ScreenQuad& Quad);
		~PostprocessAutoExposure() = default;

		Texture* Draw(float Exposure, Texture* Frame, iVector2 FrameSize,
			Buffer* UBO, size_t Offset, size_t Size);
	};

}


