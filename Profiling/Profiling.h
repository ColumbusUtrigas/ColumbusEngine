#pragma once

#include <Core/Types.h>
#include <Core/Timer.h>

namespace Columbus
{

	enum class ProfileModule
	{
		CPU,
		Audio,
		Physics,
		Particles,
		Culling,
		Update
	};

	enum class ProfileModuleGPU
	{
		GPU,
		OpaqueStage,
		SkyStage,
		TransparentStage,
		BloomStage,
		FinalStage
	};

	struct ProfileMarker
	{
		ProfileModule Module;
		Timer Prof;

		ProfileMarker(ProfileModule Module);
		~ProfileMarker();
	};

	struct ProfileMarkerGPU
	{
		ProfileModuleGPU Module;
		int64 Start, End;

		ProfileMarkerGPU(ProfileModuleGPU Module);
		~ProfileMarkerGPU();
	};

	#define PROFILE_CPU(module) ProfileMarker MarkerCPU(module);
	#define PROFILE_GPU(module) ProfileMarkerGPU MarkerGPU(module);

	void ResetProfiling();	
	double GetProfileTime(ProfileModule Module);
	double GetProfileTimeGPU(ProfileModuleGPU Module);

}


