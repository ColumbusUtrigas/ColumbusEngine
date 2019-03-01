#pragma once

#include <Graphics/Particles/ParticleEmitterCPU.h>
#include <Graphics/Material.h>
#include <Graphics/OpenGL/BufferOpenGL.h>
#include <Graphics/OpenGL/ShaderOpenGL.h>

namespace Columbus
{

	class ParticlesRenderer
	{
	private:
		size_t MaxSize = 0;

		char* Data = nullptr;

		BufferOpenGL VerticesBuffer;
		BufferOpenGL TexcoordsBuffer;
		BufferOpenGL PositionsBuffer;
		BufferOpenGL SizesBuffer;
		BufferOpenGL ColorsBuffer;
		BufferOpenGL OtherDataBuffer;

		ShaderProgramOpenGL* Shader;
	private:
		void Allocate(size_t NewSize);
	public:
		ParticlesRenderer(size_t MaxSize = 1024);

		void Render(const ParticleEmitterCPU& Particles, const Camera& MainCamera, const Material& Mat);

		~ParticlesRenderer();
	};

}


