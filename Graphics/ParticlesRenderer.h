#pragma once

#include <Graphics/Particles/ParticleEmitterCPU.h>
#include <Graphics/Material.h>
#include <Graphics/OpenGL/BufferOpenGL.h>

namespace Columbus
{

	class ParticlesRenderer
	{
	private:
		size_t MaxSize = 0;

		BufferOpenGL PositionsBuffer;
		BufferOpenGL SizesBuffer;
		BufferOpenGL ColorsBuffer;
		BufferOpenGL OtherDataBuffer;
	private:
		void Allocate(size_t NewSize);
	public:
		ParticlesRenderer(size_t MaxSize = 1024);

		void Render(const ParticleEmitterCPU& Particles, const Camera& MainCamera, const Material& Mat);

		~ParticlesRenderer();
	};

}


