#pragma once

#include <Math/Vector3.h>

#include <Graphics/OpenGL/BufferOpenGL.h>
#include <Graphics/ScreenQuad.h>

namespace Columbus
{

	class PostprocessVignette
	{
	private:
		ScreenQuad& _Quad;
	public:
		bool Enabled = false;
		Vector3 Color;
		Vector2 Center{0.5f};
		float Intensity = 1.0f;
		float Smoothness = 0.2f;
		float Radius = 0.6f;
	public:
		PostprocessVignette(ScreenQuad& Quad) :
			_Quad(Quad) {}

		// Offset and Size are values in the UBO
		void Draw(const BufferOpenGL& UBO, size_t Offset, size_t Size);
	};

}


