#include <Profiling/Profiling.h>
#include <Math/MathUtil.h>

#include <GL/glew.h>
#include <string.h>
#include <vector>
#include <unordered_map>
#include <string_view>

namespace Columbus
{

	std::vector<ProfileCounterCPU*>& GetListCPU()
	{
		static std::vector<ProfileCounterCPU*> CPU;
		return CPU;
	}

	std::vector<const char*>& GetCategoriesListCPU()
	{
		static std::vector<const char*> List;
		return List;
	}

	void AddCategoryToCategoriesList(const char* Category)
	{
		auto& Categories = GetCategoriesListCPU();
		for (const char* Cat : Categories)
		{
			if (strcmp(Category, Cat) == 0)
				return;
		}

		Categories.push_back(Category);
	}

	std::vector<ProfileCounterCPU*>& GetCategoryListCPU(const char* Category)
	{
		static std::unordered_map<std::string_view, std::vector<ProfileCounterCPU*>> Map;
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
		Id = (int)GetListCPU().size();
		GetListCPU().push_back(this);
		GetCategoryListCPU(Category).push_back(this);
		AddCategoryToCategoriesList(Category);
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

	ProfileMarkerGPU::ProfileMarkerGPU(ProfileModuleGPU Module) : Module(Module)
	{
		//glGenQueries(1, &ID);
		//glBeginQuery(GL_TIME_ELAPSED, ID);
	}

	ProfileMarkerGPU::~ProfileMarkerGPU()
	{
		GLint result;
		//glEndQuery(GL_TIME_ELAPSED);
		//glGetQueryObjectiv(ID, GL_QUERY_RESULT, &result);
		//glDeleteQueries(1, &ID);

		//GPU[int(Module)] = result * 0.000001;
	}

	void ResetProfiling()
	{
		for (ProfileCounterCPU* Counter : GetListCPU())
		{
			Counter->Time = 0.0;
		}
		// memset(CPU, 0, sizeof(CPU));
		// memset(GPU, 0, sizeof(GPU));
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
		return GetCategoriesListCPU();
	}

	std::span<ProfileCounterCPU*> GetProfilerCategoryCPU(const char* Category)
	{
		return GetCategoryListCPU(Category);
	}

}
