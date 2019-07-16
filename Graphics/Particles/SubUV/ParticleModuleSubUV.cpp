#include <Graphics/Particles/SubUV/ParticleModuleSubUV.h>
#include <Core/Platform.h>

#if defined(PLATFORM_WINDOWS)
	#include <intrin.h>
#elif defined(PLATFORM_LINUX)
	#include <x86intrin.h>
#endif

namespace Columbus
{

	void ParticleModuleSubUV::Spawn(ParticleContainer& Container, size_t Spawn) const
	{
		switch (Mode)
		{
		case SubUVMode::Linear:
			for (size_t i = Container.Count; i < Container.Count + Spawn; i++)
			{
				Container.Frames[i] = 0;
			}
			break;
		case SubUVMode::Random:
			for (size_t i = Container.Count; i < Container.Count + Spawn; i++)
			{
				Container.Frames[i] = Random::Range<uint32>(0, Horizontal * Vertical);
			}
			break;
		}
	}

	void ParticleModuleSubUV::Update(ParticleContainer& Container) const
	{
	#if USE_SIMD_EXTENSIONS
		if (Mode == SubUVMode::Linear)
		{
			float M = Horizontal * Vertical * Cycles;
			__m128 Factor = _mm_load1_ps(&M);
			for (size_t i = 0, id = 0; i < (Container.Count + 3) / 4; i++, id += 4)
			{
				__m128 Percents = _mm_load_ps(&Container.Percents[id]);
				__m128 Frames = _mm_mul_ps(Factor, Percents);
				__m128i FramesInt = _mm_cvtps_epi32(Frames);
				_mm_store_si128((__m128i*)&Container.Frames[id], FramesInt);
			}
		}
	#else
		if (Mode == SubUVMode::Linear)
		{
			for (size_t i = 0; i < Container.Count; i++)
			{
				Container.Frames[i] = static_cast<uint32>(floorf(Horizontal * Vertical * Container.Percents[i] * Cycles));
			}
		}
	#endif
	}

}


