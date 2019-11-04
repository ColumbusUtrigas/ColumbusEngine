#include <Profiling/Profiling.h>

#include <GL/glew.h>

namespace Columbus
{

	struct
	{
		double CPUTime;
		double AudioTime;
		double PhysicsTime;
		double ParticlesTime;
		double CullingTime;
		double UpdateTime;
	} CPU;

	struct
	{
		double GPUTime;
		double OpaqueStageTime;
		double SkyStageTime;
		double TransparentStageTime;
		double BloomStageTime;
		double FinalStageTime;
	} GPU;

	ProfileMarker::ProfileMarker(ProfileModule Module) : Module(Module)
	{
		Prof.Reset();
	}

	ProfileMarker::~ProfileMarker()
	{
		double Time = Prof.Elapsed() * 1000;

		switch (Module)
		{
		case ProfileModule::CPU:       CPU.CPUTime        = Time; break;
		case ProfileModule::Audio:     CPU.AudioTime      = Time; break;
		case ProfileModule::Physics:   CPU.PhysicsTime    = Time; break;
		case ProfileModule::Particles: CPU.ParticlesTime += Time; break;
		case ProfileModule::Culling:   CPU.CullingTime    = Time; break;
		case ProfileModule::Update:    CPU.UpdateTime     = Time; break;
		}
	}

	ProfileMarkerGPU::ProfileMarkerGPU(ProfileModuleGPU Module) : Module(Module)
	{
		glGetInteger64v(GL_TIMESTAMP, &Start);
	}

	ProfileMarkerGPU::~ProfileMarkerGPU()
	{
		glGetInteger64v(GL_TIMESTAMP, &End);
		double Time = (End - Start) * 0.000001;

		switch (Module)
		{
		case ProfileModuleGPU::GPU:              GPU.GPUTime              = Time; break;
		case ProfileModuleGPU::OpaqueStage:      GPU.OpaqueStageTime      = Time; break;
		case ProfileModuleGPU::SkyStage:         GPU.SkyStageTime         = Time; break;
		case ProfileModuleGPU::TransparentStage: GPU.TransparentStageTime = Time; break;
		case ProfileModuleGPU::BloomStage:       GPU.BloomStageTime       = Time; break;
		case ProfileModuleGPU::FinalStage:       GPU.FinalStageTime       = Time; break;
		}
	}

	void ResetProfiling()
	{
		//CPU.CPUTime = 0.0;
		//CPU.AudioTime = 0.0;
		CPU.PhysicsTime = 0.0;
		CPU.ParticlesTime = 0.0;
		CPU.CullingTime = 0.0;
		CPU.UpdateTime = 0.0;

		GPU.GPUTime = 0.0;
		GPU.OpaqueStageTime = 0.0;
		GPU.SkyStageTime = 0.0;
		GPU.TransparentStageTime = 0.0;
		GPU.BloomStageTime = 0.0;
		GPU.FinalStageTime = 0.0;
	}

	double GetProfileTime(ProfileModule Module)
	{
		switch (Module)
		{
		case ProfileModule::CPU:       return CPU.CPUTime;       break;
		case ProfileModule::Audio:     return CPU.AudioTime;     break;
		case ProfileModule::Physics:   return CPU.PhysicsTime;   break;
		case ProfileModule::Particles: return CPU.ParticlesTime; break;
		case ProfileModule::Culling:   return CPU.CullingTime;   break;
		case ProfileModule::Update:    return CPU.UpdateTime;    break;
		}

		return 0.0;
	}

	double GetProfileTimeGPU(ProfileModuleGPU Module)
	{
		switch (Module)
		{
		case ProfileModuleGPU::GPU:              return GPU.GPUTime;              break;
		case ProfileModuleGPU::OpaqueStage:      return GPU.OpaqueStageTime;      break;
		case ProfileModuleGPU::SkyStage:         return GPU.SkyStageTime;         break;
		case ProfileModuleGPU::TransparentStage: return GPU.TransparentStageTime; break;
		case ProfileModuleGPU::BloomStage:       return GPU.BloomStageTime;       break;
		case ProfileModuleGPU::FinalStage:       return GPU.FinalStageTime;       break;
		}

		return 0.0;
	}

}


