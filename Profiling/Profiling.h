#pragma once

#include <Core/Types.h>
#include <Core/Timer.h>
#include <span>

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

	// to be used with a macro, should be statically initialised
	struct ProfileCounterCPU
	{
		int Id;
		double Time;
		const char* Text;
		const char* Category;

		ProfileCounterCPU(const char* Text, const char* Category);
	};

	struct ProfileMarkerScopedCPU
	{
		ProfileCounterCPU& Marker;
		Timer Prof;

		ProfileMarkerScopedCPU(ProfileCounterCPU& Marker);
		~ProfileMarkerScopedCPU();
	};


	struct ProfileMarkerGPU
	{
		ProfileModuleGPU Module;
		uint32 ID;

		ProfileMarkerGPU(ProfileModuleGPU Module);
		~ProfileMarkerGPU();
	};

	//#define PROFILE_CPU(module) Columbus::ProfileMarker MarkerCPU(module);
 	#define PROFILE_GPU(module) Columbus::ProfileMarkerGPU MarkerGPU(module);
 
	#define DECLARE_CPU_PROFILING_COUNTER(counter) extern Columbus::ProfileCounterCPU counter;
	#define IMPLEMENT_CPU_PROFILING_COUNTER(text, category, counter) Columbus::ProfileCounterCPU counter (text, category);

	#define PROFILE_CPU(counter) Columbus::ProfileMarkerScopedCPU MarkerCPU(counter);

 	void ResetProfiling();	
 	double GetProfileTime(ProfileModule Module);
 	double GetProfileTimeGPU(ProfileModuleGPU Module);
 
	std::span<const char*>        GetProfilerCategoryListCPU();
	std::span<ProfileCounterCPU*> GetProfilerCategoryCPU(const char* Category);

	void ResetProfiling();	
	double GetProfileTime(ProfileModule Module);
	double GetProfileTimeGPU(ProfileModuleGPU Module);

}


