#include <Profiling/Profiling.h>
#include <Math/MathUtil.h>

#include <GL/glew.h>
#include <string.h>
#include <vector>
#include <unordered_map>
#include <string_view>

namespace Columbus
{

	template <typename T>
	std::vector<T*>& GetCounterList()
	{
		static std::vector<T*> List;
		return List;
	}

	template <typename T>
	std::vector<const char*>& GetCategoriesList()
	{
		static std::vector<const char*> List;
		return List;
	}

	template <typename T>
	void AddCategoryToCategoriesList(const char* Category)
	{
		auto& Categories = GetCategoriesList<T>();
		for (const char* Cat : Categories)
		{
			if (strcmp(Category, Cat) == 0)
				return;
		}

		Categories.push_back(Category);
	}

	template <typename T>
	std::vector<T*>& GetCategoryList(const char* Category)
	{
		static std::unordered_map<std::string_view, std::vector<T*>> Map;
		if (!Map.contains(Category))
		{
			Map[Category] = {};
		}

		return Map[Category];
	}


	// double CPU[int(ProfileModule::Count)];
	double GPU[int(ProfileModuleGPU::Count)];

	ProfileCounterCPU::ProfileCounterCPU(const char* Text, const char* Category) : Text(Text), Category(Category)
	{
		Time = 0;
		LastTime = 0;
		Id = (int)GetCounterList<ProfileCounterCPU>().size();
		GetCounterList<ProfileCounterCPU>().push_back(this);
		GetCategoryList<ProfileCounterCPU>(Category).push_back(this);
		AddCategoryToCategoriesList<ProfileCounterCPU>(Category);
	}

	ProfileCounterMemory::ProfileCounterMemory(const char* Text, const char* Category) : Text(Text), Category(Category)
	{
		Memory = 0;
		Id = (int)GetCounterList<ProfileCounterMemory>().size();
		GetCounterList<ProfileCounterMemory>().push_back(this);
		GetCategoryList<ProfileCounterMemory>(Category).push_back(this);
		AddCategoryToCategoriesList<ProfileCounterMemory>(Category);
 	}

	ProfileCounterGPU::ProfileCounterGPU(const char* Text, const char* Category) : Text(Text), Category(Category)
	{
		Time = 0;
		LastTime = 0;
		Id = (int)GetCounterList<ProfileCounterGPU>().size();
		GetCounterList<ProfileCounterGPU>().push_back(this);
		GetCategoryList<ProfileCounterGPU>(Category).push_back(this);
		AddCategoryToCategoriesList<ProfileCounterGPU>(Category);
	}

	ProfileCounterCounting::ProfileCounterCounting(const char* Text, const char* Category, const bool ResetEveryFrame)
		: Text(Text), Category(Category), ResetEveryFrame(ResetEveryFrame)
	{
		Count = 0;
		Id = (int)GetCounterList<ProfileCounterCounting>().size();
		GetCounterList<ProfileCounterCounting>().push_back(this);
		GetCategoryList<ProfileCounterCounting>(Category).push_back(this);
		AddCategoryToCategoriesList<ProfileCounterCounting>(Category);
	}

	ProfileMarkerScopedCPU::ProfileMarkerScopedCPU(ProfileCounterCPU& Marker) : Marker(Marker)
	{
		Prof.Reset();
	}

	ProfileMarkerScopedCPU::~ProfileMarkerScopedCPU()
	{
		Marker.Time += Prof.Elapsed() * 1000;
	}


	ProfileMarker::ProfileMarker(ProfileModule Module) : Module(Module)
	{
		Prof.Reset();
	}

	ProfileMarker::~ProfileMarker()
	{
		// double Time = Prof.Elapsed() * 1000;
		// CPU[int(Module)] += Time;
	}

	void ResetProfiling()
	{
		for (ProfileCounterCPU* Counter : GetCounterList<ProfileCounterCPU>())
		{
			Counter->LastTime = Counter->Time;
			Counter->Time = 0.0;
		}

		for (ProfileCounterGPU* Counter : GetCounterList<ProfileCounterGPU>())
		{
			Counter->LastTime = Counter->Time;
			Counter->Time = 0.0;
		}

		for (ProfileCounterCounting* Counter : GetCounterList<ProfileCounterCounting>())
		{
			if (Counter->ResetEveryFrame)
			{
				Counter->Count = 0;
			}
		}
		// memset(CPU, 0, sizeof(CPU));
		// memset(GPU, 0, sizeof(GPU));
	}

	void AddProfilingMemory(ProfileCounterMemory& Counter, u64 Bytes)
	{
		Counter.Memory += Bytes;
	}

	void RemoveProfilingMemory(ProfileCounterMemory& Counter, u64 Bytes)
	{
		Counter.Memory -= Bytes;
	}

	void SetProfilingMemory(ProfileCounterMemory& Counter, u64 Bytes)
	{
		Counter.Memory = Bytes;
	}

	void AddProfilingCount(ProfileCounterCounting& Counter, u64 Count)
	{
		Counter.Count += Count;
	}

	void RemoveProfilingCount(ProfileCounterCounting& Counter, u64 Count)
	{
		Counter.Count -= Count;
	}

	void SetProfilingCount(ProfileCounterCounting& Counter, u64 Count)
	{
		Counter.Count = Count;
	}

	double GetProfileTime(ProfileModule Module)
	{
		return 0.0;
		// return CPU[int(Module)];
	}

	double GetProfileTimeGPU(ProfileModuleGPU Module)
	{
		return GPU[int(Module)];
	}

	std::span<const char*> GetProfilerCategoryListCPU()
	{
		return GetCategoriesList<ProfileCounterCPU>();
	}

	std::span<ProfileCounterCPU*> GetProfilerCategoryCPU(const char* Category)
	{
		return GetCategoryList<ProfileCounterCPU>(Category);
	}

	std::span<const char*> GetProfilerCategoryListMemory()
	{
		return GetCategoriesList<ProfileCounterMemory>();
	}

	std::span<ProfileCounterMemory*> GetProfilerCategoryMemory(const char* Category)
	{
		return GetCategoryList<ProfileCounterMemory>(Category);
 	}

	std::span<const char*> GetProfilerCategoryListGPU()
	{
		return GetCategoriesList<ProfileCounterGPU>();
	}

	std::span<ProfileCounterGPU*> GetProfilerCategoryGPU(const char* Category)
	{
		return GetCategoryList<ProfileCounterGPU>(Category);
	}

	std::span<const char*> GetProfilerCategoryListCounting()
	{
		return GetCategoriesList<ProfileCounterCounting>();
	}

	std::span<ProfileCounterCounting*> GetProfilerCategoryCounting(const char* Category)
	{
		return GetCategoryList<ProfileCounterCounting>(Category);
	}

}
