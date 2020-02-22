#include <Profiling/Profiling.h>
#include <Math/MathUtil.h>

#include <GL/glew.h>

namespace Columbus
{

	double CPU[int(ProfileModule::Count)];
	double GPU[int(ProfileModuleGPU::Count)];

	ProfileMarker::ProfileMarker(ProfileModule Module) : Module(Module)
	{
		Prof.Reset();
	}

	ProfileMarker::~ProfileMarker()
	{
		double Time = Prof.Elapsed() * 1000;
		CPU[int(Module)] += Time;
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
		memset(CPU, 0, sizeof(CPU));
		memset(GPU, 0, sizeof(GPU));
	}

	double GetProfileTime(ProfileModule Module)
	{
		return CPU[int(Module)];
	}

	double GetProfileTimeGPU(ProfileModuleGPU Module)
	{
		return GPU[int(Module)];
	}

}
