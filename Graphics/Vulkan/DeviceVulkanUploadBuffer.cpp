#include "DeviceVulkan.h"
#include "Counters.h"

IMPLEMENT_MEMORY_PROFILING_COUNTER("UploadRing Size Total", PROFILING_CATEGORY_VULKAN_LOW_LEVEL, MemoryCounter_Vulkan_UploadRingSize);
IMPLEMENT_MEMORY_PROFILING_COUNTER("UploadRing Size Current", PROFILING_CATEGORY_VULKAN_LOW_LEVEL, MemoryCounter_Vulkan_UploadRingUploadsSize);

IMPLEMENT_CPU_PROFILING_COUNTER("UploadRing Flush Time", PROFILING_CATEGORY_VULKAN_LOW_LEVEL, CpuCounter_Vulkan_UploadRingFlushTime, true);

IMPLEMENT_COUNTING_PROFILING_COUNTER("UploadRing Flushes Count", PROFILING_CATEGORY_VULKAN_LOW_LEVEL, CountingCounter_Vulkan_UploadRingFlushes, true);
IMPLEMENT_COUNTING_PROFILING_COUNTER("UploadRing Uploads Count", PROFILING_CATEGORY_VULKAN_LOW_LEVEL, CountingCounter_Vulkan_UploadRingUploads, true);

namespace Columbus
{

	void HUploadBufferRing::UploadBuffer(const void* Data, u32 Size, u32 DstOffset, Buffer* Buf)
	{
		void* Ptr = UploadBufferMap(Size, DstOffset, Buf);
		memcpy(Ptr, Data, Size);
	}

	void* HUploadBufferRing::UploadBufferMap(u32 Size, u32 DstOffset, Buffer* Buf)
	{
		u32 CurrentSize = UploadBuffers[CurrentFrame]->GetDesc().Size;
		if (UploadOffsets[CurrentFrame] + Size >= CurrentSize)
		{
			DEBUGBREAK();
			FlushUploads();
			Realloc(CurrentFrame, CurrentSize * DefaultGrowthFactor);
		}

		u8* ptr = MappedBuffers[CurrentFrame];
		u32 offset = UploadOffsets[CurrentFrame];

		UploadEntry Entry
		{
			.Dst = Buf,
			.HostOffset = UploadOffsets[CurrentFrame],
			.DstOffset = DstOffset,
			.Size = Size
		};
		ScheduledUploads[CurrentFrame].push_back(Entry);

		UploadOffsets[CurrentFrame] += Size;

		AddProfilingMemory(MemoryCounter_Vulkan_UploadRingUploadsSize, Size);
		AddProfilingCount(CountingCounter_Vulkan_UploadRingUploads, 1);

		return ptr + offset;
	}

	void HUploadBufferRing::FlushUploads()
	{
		PROFILE_CPU(CpuCounter_Vulkan_UploadRingFlushTime);
		AddProfilingCount(CountingCounter_Vulkan_UploadRingFlushes, 1);

		UploadOffsets[CurrentFrame] = 0;

		if (ScheduledUploads[CurrentFrame].empty())
			return;

		Device->WaitForFence(UploadFences[CurrentFrame], UINT64_MAX);

		CommandBufferVulkan* CmdBuf = CommandBufs[CurrentFrame];
		CmdBuf->Reset();
		CmdBuf->Begin();
		CmdBuf->BeginDebugMarker("Flush UploadBufferRing");

		while (!ScheduledUploads[CurrentFrame].empty())
		{
			const UploadEntry Entry = ScheduledUploads[CurrentFrame].back();

			CmdBuf->CopyBuffer(UploadBuffers[CurrentFrame], Entry.Dst, Entry.HostOffset, Entry.DstOffset, Entry.Size);

			ScheduledUploads[CurrentFrame].pop_back();
		}

		CmdBuf->EndDebugMarker();
		CmdBuf->End();

		Device->Submit(CmdBuf, UploadFences[CurrentFrame], 0, nullptr, 0, nullptr);
	}


	void HUploadBufferRing::BeginFrame()
	{
		CurrentFrame = (CurrentFrame + 1) % MaxFramesInFlight;
		SetProfilingMemory(MemoryCounter_Vulkan_UploadRingUploadsSize, 0);

		COLUMBUS_ASSERT(ScheduledUploads[CurrentFrame].empty());
	}

	void HUploadBufferRing::Init()
	{
		for (u32 i = 0; i < MaxFramesInFlight; i++)
		{
			UploadFences[i] = Device->CreateFence(true);

			Realloc(i, DefaultUploadSize);
			CommandBufs[i] = Device->CreateCommandBuffer();
		}
	}

	void HUploadBufferRing::Realloc(int BufNum, u32 NewSize)
	{
		BufferDesc Desc;
		Desc.HostVisible = true;
		Desc.Size = NewSize;

		if (UploadBuffers[BufNum])
		{
			FlushUploads();

			RemoveProfilingMemory(MemoryCounter_Vulkan_UploadRingSize, UploadBuffers[BufNum]->GetDesc().Size);

			Device->UnmapBuffer(UploadBuffers[BufNum]); // unmap before destroying
			Device->DestroyBuffer(UploadBuffers[BufNum]);
		}

		AddProfilingMemory(MemoryCounter_Vulkan_UploadRingSize, NewSize);

		UploadBuffers[BufNum] = Device->CreateBuffer(Desc, nullptr);
		MappedBuffers[BufNum] = (u8*)Device->MapBuffer(UploadBuffers[BufNum]);

		Device->SetDebugName(UploadBuffers[BufNum], "UploadRingBuffer");
	}

	void HUploadBufferRing::Shutdown()
	{
		for (u32 i = 0; i < MaxFramesInFlight; i++)
		{
			Device->UnmapBuffer(UploadBuffers[i]);
			Device->DestroyBuffer(UploadBuffers[i]);
		}
	}

}
