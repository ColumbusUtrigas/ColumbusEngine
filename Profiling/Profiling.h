#pragma once

#include <Core/Types.h>
#include <Core/Timer.h>
#include <span>

namespace Columbus
{

	// TODO: legacy
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

	// TODO: legacy
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

	// TODO: legacy
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
		double LastTime;
		const char* Text;
		const char* Category;

		ProfileCounterCPU(const char* Text, const char* Category);
	};

	// to be used with a macro, should be statically initialised
	struct ProfileCounterMemory
	{
		int Id;
		u64 Memory;
		const char* Text;
		const char* Category;

		ProfileCounterMemory(const char* Text, const char* Category);
	};

	// to be used with a macro, should be statically initialised
	// doesn't measure GPU time by itself
	// it's just a data holder, data is written to here from GPUProfiler
	struct ProfileCounterGPU
	{
		int Id;
		double Time;
		double LastTime;
		const char* Text;
		const char* Category;

		ProfileCounterGPU(const char* Text, const char* Category);
	};

	// to be used with a macro, should be statically initialised
	struct ProfileCounterCounting
	{
		int Id;
		u64 Count;

		const char* Text;
		const char* Category;
		const bool ResetEveryFrame;

		ProfileCounterCounting(const char* Text, const char* Category, const bool ResetEveryFrame);
	};

	struct ProfileMarkerScopedCPU
	{
		ProfileCounterCPU& Marker;
		Timer Prof;

		ProfileMarkerScopedCPU(ProfileCounterCPU& Marker);
		~ProfileMarkerScopedCPU();
	};

	// TODO: legacy
	//#define PROFILE_CPU(module) Columbus::ProfileMarker MarkerCPU(module);
 	// #define PROFILE_GPU(module) Columbus::ProfileMarkerGPU MarkerGPU(module);
 
	#define DECLARE_CPU_PROFILING_COUNTER(counter) extern Columbus::ProfileCounterCPU counter;
	#define IMPLEMENT_CPU_PROFILING_COUNTER(text, category, counter) Columbus::ProfileCounterCPU counter (text, category);

	#define DECLARE_MEMORY_PROFILING_COUNTER(counter) extern Columbus::ProfileCounterMemory counter;
	#define IMPLEMENT_MEMORY_PROFILING_COUNTER(text, category, counter) Columbus::ProfileCounterMemory counter (text, category);

	#define DECLARE_GPU_PROFILING_COUNTER(counter) extern Columbus::ProfileCounterGPU counter;
	#define IMPLEMENT_GPU_PROFILING_COUNTER(text, category, counter) Columbus::ProfileCounterGPU counter (text, category);

	#define DECLARE_COUNTING_PROFILING_COUNTER(counter) extern Columbus::ProfileCounterCounting counter;
	#define IMPLEMENT_COUNTING_PROFILING_COUNTER(text, category, counter, resetEveryFrame) Columbus::ProfileCounterCounting counter (text, category, resetEveryFrame);

	#define PROFILE_CPU(counter) Columbus::ProfileMarkerScopedCPU MarkerCPU(counter);
	// #define PROFILE_GPU(counter) // TODO: use ProfileMarkerScopedGPU

	void AddProfilingMemory(ProfileCounterMemory& Counter, u64 Bytes);
	void RemoveProfilingMemory(ProfileCounterMemory& Counter, u64 Bytes);
	void SetProfilingMemory(ProfileCounterMemory& Counter, u64 Bytes);

	void AddProfilingCount(ProfileCounterCounting& Counter, u64 Count);
	void RemoveProfilingCount(ProfileCounterCounting& Counter, u64 Count);
	void SetProfilingCount(ProfileCounterCounting& Counter, u64 Count);

	// call it once per frame
 	void ResetProfiling();

	// TODO: legacy
 	double GetProfileTime(ProfileModule Module);
 	double GetProfileTimeGPU(ProfileModuleGPU Module);
 
	std::span<const char*>        GetProfilerCategoryListCPU();
	std::span<ProfileCounterCPU*> GetProfilerCategoryCPU(const char* Category);

	std::span<const char*>           GetProfilerCategoryListMemory();
	std::span<ProfileCounterMemory*> GetProfilerCategoryMemory(const char* Category);

	std::span<const char*>        GetProfilerCategoryListGPU();
	std::span<ProfileCounterGPU*> GetProfilerCategoryGPU(const char* Category);

	std::span<const char*>             GetProfilerCategoryListCounting();
	std::span<ProfileCounterCounting*> GetProfilerCategoryCounting(const char* Category);

	// TODO: legacy
	void ResetProfiling();	
	double GetProfileTime(ProfileModule Module);
	double GetProfileTimeGPU(ProfileModuleGPU Module);

}
