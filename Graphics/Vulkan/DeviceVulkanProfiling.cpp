#include "DeviceVulkan.h"

namespace Columbus
{

	void GPUProfilerVulkan::Init()
	{
		for (int i = 0; i < MaxFramesInFlight; i++)
		{
			QueryPoolDesc Desc {
				.Type = QueryPoolType::Timestamp,
				.Count = TimestampQueryCount
			};

			Pools[i] = Device->CreateQueryPool(Desc);
		}
	}

	void GPUProfilerVulkan::Shutdown()
	{
		for (int i = 0; i < MaxFramesInFlight; i++)
		{
			Device->DestroyQueryPool(Pools[i]);
		}
	}

	void GPUProfilerVulkan::BeginFrame()
	{
		// Device->ResetQueryPool(Pools[CurrentFrame], 0, TimestampQueryCount);
		CurrentTimestampQuery[CurrentFrame] = 0;
		CurrentMeasurement[CurrentFrame] = 0;
		WasReset = false;
	}

	void GPUProfilerVulkan::EndFrame()
	{
		int LastFrame = (CurrentFrame + MaxFramesInFlight - 1) % MaxFramesInFlight;
		CurrentFrame++;
		CurrentFrame %= MaxFramesInFlight;

		for (const auto& Sample : ReadFrameTimings(LastFrame))
			Sample.Counter->Time += Sample.Milliseconds;
	}

	std::vector<GPUProfilerVulkan::CounterTiming> GPUProfilerVulkan::ReadFrameTimings(int FrameIndex) const
	{
		COLUMBUS_ASSERT(FrameIndex >= 0 && FrameIndex < MaxFramesInFlight);
		std::vector<CounterTiming> Result;
		const int QueryCount = CurrentTimestampQuery[FrameIndex].load();
		if (QueryCount == 0) return Result;
		u64 Data[TimestampQueryCount];
		Device->ReadQueryPoolTimestamps(Pools[FrameIndex], 0, QueryCount, Data, sizeof(Data));
		for (int i = 0; i < CurrentMeasurement[FrameIndex]; ++i)
		{
			const Measurement& Sample = Measurements[FrameIndex][i];
			const double Nanoseconds = double(Data[Sample.EndTimestampId] - Data[Sample.StartTimestampId]) * Device->_DeviceProperties.properties.limits.timestampPeriod;
			Result.push_back({Sample.Counter, Nanoseconds / 1000000.0});
		}
		return Result;
	}

	void GPUProfilerVulkan::Reset(CommandBufferVulkan* CommandBuffer)
	{
		if (!WasReset)
		{
			CommandBuffer->ResetQueryPool(Pools[CurrentFrame], 0, TimestampQueryCount);
		}
		WasReset = true;
	}

	void GPUProfilerVulkan::BeginProfileCounter(ProfileMarkerGPU& Scoped, CommandBufferVulkan* CommandBuffer)
	{
		Scoped.Id = CurrentMeasurement[CurrentFrame]++;
		Measurement& Sample = Measurements[CurrentFrame][Scoped.Id];
		Sample.Counter = Scoped.Counter;

		Sample.StartTimestampId = CurrentTimestampQuery[CurrentFrame]++;
		COLUMBUS_ASSERT(Sample.StartTimestampId < TimestampQueryCount);
		CommandBuffer->WriteTimestamp(Pools[CurrentFrame], Sample.StartTimestampId);
	}

	void GPUProfilerVulkan::EndProfileConter(ProfileMarkerGPU& Scoped, CommandBufferVulkan* CommandBuffer)
	{
		Measurement& Sample = Measurements[CurrentFrame][Scoped.Id];

		Sample.EndTimestampId = CurrentTimestampQuery[CurrentFrame]++;
		COLUMBUS_ASSERT(Sample.EndTimestampId < TimestampQueryCount);
		CommandBuffer->WriteTimestamp(Pools[CurrentFrame], Sample.EndTimestampId);
	}

	ProfileMarkerScopedGPU::ProfileMarkerScopedGPU(ProfileCounterGPU* Counter, GPUProfilerVulkan* Profiler, CommandBufferVulkan* CommandBuffer) :
		ProfileMarkerGPU(Counter), Profiler(Profiler), CommandBuffer(CommandBuffer)
	{
		Profiler->BeginProfileCounter(*this, CommandBuffer);
	}

	ProfileMarkerScopedGPU::~ProfileMarkerScopedGPU()
	{
		Profiler->EndProfileConter(*this, CommandBuffer);
	}

}
