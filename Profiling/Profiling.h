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
		Update,
		Count
	};

	enum class ProfileModuleGPU
	{
		GPU,
		OpaqueStage,
		SkyStage,
		TransparentStage,
		BloomStage,
		FinalStage,
		Count
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
		uint32 ID;

		ProfileMarkerGPU(ProfileModuleGPU Module);
		~ProfileMarkerGPU();
	};

	#define PROFILE_CPU(module) Columbus::ProfileMarker MarkerCPU(module);
	#define PROFILE_GPU(module) Columbus::ProfileMarkerGPU MarkerGPU(module);

	void ResetProfiling();	
	double GetProfileTime(ProfileModule Module);
	double GetProfileTimeGPU(ProfileModuleGPU Module);

}


