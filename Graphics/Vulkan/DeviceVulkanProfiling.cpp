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
	}

	void GPUProfilerVulkan::EndFrame()
	{
		int LastFrame = (CurrentFrame + MaxFramesInFlight - 1) % MaxFramesInFlight;
		CurrentFrame++;
		CurrentFrame %= MaxFramesInFlight;

		if (CurrentTimestampQuery[LastFrame] > 0)
		{
			u64 Data[TimestampQueryCount];
			Device->ReadQueryPoolTimestamps(Pools[LastFrame], 0, CurrentTimestampQuery[LastFrame], Data, TimestampQueryCount * sizeof(u64));

			// TODO: readback results, populate profiler with data

			for (int i = 0; i < CurrentMeasurement[LastFrame]; i++)
			{
				Measurement& Sample = Measurements[LastFrame][i];
				u64 diffNs = Data[Sample.EndTimestampId] - Data[Sample.StartTimestampId];
				u64 diffUs = diffNs / 1000;
				double diffMs = diffUs / 1000.0;

				Sample.Counter->Time += diffMs;
			}
		}
	}

	void GPUProfilerVulkan::Reset(CommandBufferVulkan* CommandBuffer)
	{
		CommandBuffer->ResetQueryPool(Pools[CurrentFrame], 0, TimestampQueryCount);
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
