#pragma once

#include <Graphics/Particles/ParticleEmitterCPU.h>
#include <Graphics/Material.h>
#include <Graphics/Device.h>

namespace Columbus
{

	class ParticlesRenderer
	{
	private:
		size_t MaxSize = 0;

		Buffer* PositionsBuffer;
		Buffer* SizesBuffer;
		Buffer* ColorsBuffer;
		Buffer* OtherDataBuffer;
	private:
		void Allocate(size_t NewSize);
	public:
		ParticlesRenderer(size_t MaxSize = 1024);

		void Render(const ParticleEmitterCPU& Particles, const Camera& MainCamera, const Material& Mat);

		~ParticlesRenderer();
	};

}


