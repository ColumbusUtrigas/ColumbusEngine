#include "BakedLighting.h"

#include "Common/Image/Image.h"
#include "Graphics/GPUScene.h"
#include "Graphics/World.h"
#include "Graphics/Vulkan/CommandBufferVulkan.h"
#include "Graphics/Vulkan/DeviceVulkan.h"
#include "System/Log.h"

#include <cstring>

namespace Columbus::BakedLighting
{
	template <typename T>
	static T* FindThingByGuid(EngineWorld& World, u64 Guid)
	{
		auto It = World.ThingGuidToId.find(Guid);
		if (It == World.ThingGuidToId.end())
			return nullptr;

		AThing** ThingPtr = World.AllThings.Get(It->second);
		if (ThingPtr == nullptr)
			return nullptr;

		return Reflection::Cast<T>(*ThingPtr);
	}

	static bool IsPowerOfTwo(int Value)
	{
		return Value > 0 && (Value & (Value - 1)) == 0;
	}

	int ClampReflectionCaptureResolution(int Resolution)
	{
		Resolution = Math::Clamp(Resolution, 32, 512);

		int Lower = 32;
		while (Lower * 2 <= Resolution)
		{
			Lower *= 2;
		}

		const int Upper = Math::Min(Lower * 2, 512);
		return (Resolution - Lower) <= (Upper - Resolution) ? Lower : Upper;
	}

	bool ValidateReflectionCaptureData(const HReflectionCaptureData& Entry)
	{
		const TextureStoredPixels& Stored = Entry.Radiance;
		const TextureFormat Format = TextureFormatFromFriendlyName(Stored.Format);
		const ImageType Type = ImageTypeFromString(Stored.Type);
		const u64 OwnerGuid = (u64)Entry.Owner.Guid;

		if (Entry.FormatVersion != ReflectionCaptureBakeFormatVersion)
		{
			Log::Warning("Skipping reflection capture bake %llu: unsupported format version %i", OwnerGuid, Entry.FormatVersion);
			return false;
		}

		if (Type != ImageType::ImageCube)
		{
			Log::Warning("Skipping reflection capture bake %llu: expected ImageCube, got %s", OwnerGuid, Stored.Type.c_str());
			return false;
		}

		if (Format != TextureFormat::BC6H)
		{
			Log::Warning("Skipping reflection capture bake %llu: expected BC6H, got %s", OwnerGuid, Stored.Format.c_str());
			return false;
		}

		if (Stored.Width <= 0 || Stored.Height <= 0 || Stored.Width != Stored.Height || !IsPowerOfTwo(Stored.Width))
		{
			Log::Warning("Skipping reflection capture bake %llu: invalid cube dimensions %ix%i", OwnerGuid, Stored.Width, Stored.Height);
			return false;
		}

		if (Stored.Depth != 1)
		{
			Log::Warning("Skipping reflection capture bake %llu: invalid cube depth %i", OwnerGuid, Stored.Depth);
			return false;
		}

		const int ExpectedMips = (int)ImageUtils::ImageFullMipCount((u32)Stored.Width, (u32)Stored.Height, (u32)Stored.Depth);
		if (Stored.Mips != ExpectedMips)
		{
			Log::Warning("Skipping reflection capture bake %llu: invalid mip count %i, expected %i", OwnerGuid, Stored.Mips, ExpectedMips);
			return false;
		}

		if (Entry.Resolution != Stored.Width || Entry.SamplesPerTexel <= 0 || Entry.Bounces <= 0)
		{
			Log::Warning("Skipping reflection capture bake %llu: invalid bake metadata", OwnerGuid);
			return false;
		}

		u64 ExpectedBytes = 0;
		if (!TextureUtils::ValidateStoredPixelByteSize(Stored, &ExpectedBytes))
		{
			Log::Warning("Skipping reflection capture bake %llu: byte size mismatch, expected %llu got %llu",
				OwnerGuid, ExpectedBytes, Stored.Pixels.Size());
			return false;
		}

		return true;
	}

	bool ReadbackIrradianceProbes(SPtr<DeviceVulkan> Device, const GPUIrradianceVolume& Volume, HIrradianceVolumeData& Data)
	{
		if (Volume.Probes == nullptr)
			return false;

		const u64 ProbeBytes = (u64)Volume.GetTotalProbes() * sizeof(GPUIrradianceProbe);
		if (ProbeBytes == 0 || Volume.Probes->GetSize() < ProbeBytes)
			return false;

		Data.Probes.Bytes.resize((size_t)ProbeBytes);
		return Device->ReadBuffer(Volume.Probes, Data.Probes.Data(), ProbeBytes);
	}

	bool ReadbackReflectionRadiance(SPtr<DeviceVulkan> Device, const GPUReflectionCapture& Capture, HReflectionCaptureData& Data)
	{
		if (Capture.Radiance == nullptr)
			return false;

		const TextureDesc2& Desc = Capture.Radiance->GetDesc();
		if (Desc.Type != TextureType::TextureCube || Desc.Format != TextureFormat::RGBA32F)
		{
			Log::Error("Reflection capture readback failed: unexpected bake texture layout");
			return false;
		}

		Image HDR;
		HDR.AllocImage(Desc.Width, Desc.Height, 1, Desc.Mips, TextureFormat::RGBA32F, ImageType::ImageCube);
		if (HDR.Data == nullptr)
		{
			Log::Error("Reflection capture readback failed: could not allocate %ux%u cube image", Desc.Width, Desc.Height);
			return false;
		}

		const u64 ExpectedBytes = HDR.GetFullSize();

		BufferDesc ReadbackDesc;
		ReadbackDesc.HostVisible = true;
		ReadbackDesc.Size = ExpectedBytes;
		Buffer* ReadbackBuffer = Device->CreateBuffer(ReadbackDesc, nullptr);
		if (ReadbackBuffer == nullptr)
		{
			Log::Error("Reflection capture readback failed: could not allocate readback buffer (%llu bytes)", ExpectedBytes);
			return false;
		}

		SPtr<CommandBufferVulkan> CommandBuffer = Device->CreateCommandBufferShared();
		SPtr<FenceVulkan> Fence = Device->CreateFence(false);

		CommandBuffer->Begin();
		CommandBuffer->TransitionImageLayout(Capture.Radiance, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
		CommandBuffer->CopyImageToBufferAllSubresources(Capture.Radiance, ReadbackBuffer);
		CommandBuffer->TransitionImageLayout(Capture.Radiance, VK_IMAGE_LAYOUT_GENERAL);
		CommandBuffer->End();

		Device->Submit(CommandBuffer.get(), Fence, 0, nullptr, 0, nullptr);
		Device->WaitForFence(Fence, UINT64_MAX);

		void* SourceData = Device->MapBuffer(ReadbackBuffer);
		memcpy(HDR.Data, SourceData, (size_t)ExpectedBytes);
		Device->UnmapBuffer(ReadbackBuffer);
		Device->DestroyBuffer(ReadbackBuffer);

		Image Compressed;
		ImageCompression::CompressionParams CompressionParams;
		CompressionParams.Format = TextureFormat::BC6H;
		if (!ImageCompression::CompressImage(HDR, Compressed, CompressionParams))
		{
			Log::Error("Reflection capture readback failed: BC6H compression failed");
			return false;
		}

		TextureUtils::StoreImagePixels(Data.Radiance, Compressed);

		u64 StoredExpectedBytes = 0;
		if (!TextureUtils::ValidateStoredPixelByteSize(Data.Radiance, &StoredExpectedBytes))
		{
			Log::Error("Reflection capture readback failed: stored byte size mismatch, expected %llu got %llu",
				StoredExpectedBytes, Data.Radiance.Pixels.Size());
			Data.Radiance = {};
			return false;
		}

		return true;
	}

	void Apply(EngineWorld& World, const HLevelLightingData& LightingData)
	{
		if (World.SceneGPU == nullptr)
			return;

		for (const HIrradianceVolumeData& Entry : LightingData.IrradianceVolumes)
		{
			const u64 OwnerGuid = (u64)Entry.Owner.Guid;

			if (Entry.FormatVersion != IrradianceVolumeBakeFormatVersion || Entry.ProbeStride != sizeof(GPUIrradianceProbe))
			{
				Log::Warning("Skipping irradiance volume bake %llu: unsupported format/stride", OwnerGuid);
				continue;
			}

			AIrradianceVolume* Thing = FindThingByGuid<AIrradianceVolume>(World, OwnerGuid);
			if (Thing == nullptr)
				continue;

			GPUIrradianceVolume* Volume = World.SceneGPU->IrradianceVolumes.Get(Thing->IrradianceVolumeHandle);
			if (Volume == nullptr)
				continue;

			// the probe buffer layout must match the current authoring grid, otherwise the bake is stale
			if (Entry.ProbeCountX != Volume->ProbeCountX ||
			    Entry.ProbeCountY != Volume->ProbeCountY ||
			    Entry.ProbeCountZ != Volume->ProbeCountZ)
			{
				Log::Warning("Skipping irradiance volume bake %llu: probe grid mismatch", OwnerGuid);
				continue;
			}

			const u64 ExpectedSize = (u64)Volume->GetTotalProbes() * sizeof(GPUIrradianceProbe);
			if (ExpectedSize == 0 || Entry.Probes.Size() != ExpectedSize)
			{
				Log::Warning("Skipping irradiance volume bake %llu: probe data size mismatch, expected %llu got %llu",
					OwnerGuid, ExpectedSize, Entry.Probes.Size());
				continue;
			}

			World.SceneGPU->SetIrradianceProbes(*Volume, Entry.Probes.Data(), ExpectedSize);
		}

		for (const HReflectionCaptureData& Entry : LightingData.ReflectionCaptures)
		{
			const u64 OwnerGuid = (u64)Entry.Owner.Guid;

			if (!ValidateReflectionCaptureData(Entry))
				continue;

			AReflectionCaptureBox* Thing = FindThingByGuid<AReflectionCaptureBox>(World, OwnerGuid);
			if (Thing == nullptr)
				continue;

			GPUReflectionCapture* Capture = World.SceneGPU->ReflectionCaptures.Get(Thing->ReflectionCaptureHandle);
			if (Capture == nullptr)
				continue;

			Image CubeView;
			TextureUtils::SetBorrowedImageViewFromStoredPixels(CubeView, Entry.Radiance);
			const bool bApplied = World.SceneGPU->SetReflectionRadiance(*Capture, CubeView);
			CubeView.Data = nullptr;

			if (!bApplied)
			{
				Log::Warning("Skipping reflection capture bake %llu: failed to create runtime texture", OwnerGuid);
			}
		}
	}
}

using namespace Columbus;

CREFLECT_STRUCT_BEGIN(HIrradianceVolumeData, "")
	CREFLECT_STRUCT_FIELD_THINGREF(AIrradianceVolume, Owner, "")
	CREFLECT_STRUCT_FIELD(int, FormatVersion, "")
	CREFLECT_STRUCT_FIELD(int, ProbeCountX, "")
	CREFLECT_STRUCT_FIELD(int, ProbeCountY, "")
	CREFLECT_STRUCT_FIELD(int, ProbeCountZ, "")
	CREFLECT_STRUCT_FIELD(int, ProbeStride, "")
	CREFLECT_STRUCT_FIELD(int, RaysPerProbe, "")
	CREFLECT_STRUCT_FIELD(int, Bounces, "")
	CREFLECT_STRUCT_FIELD(Vector3, Position, "Noedit")
	CREFLECT_STRUCT_FIELD(Vector3, Extent, "Noedit")
	CREFLECT_STRUCT_FIELD(Blob, Probes, "Noedit")
CREFLECT_STRUCT_END()

CREFLECT_STRUCT_BEGIN(HReflectionCaptureData, "")
	CREFLECT_STRUCT_FIELD_THINGREF(AReflectionCaptureBox, Owner, "")
	CREFLECT_STRUCT_FIELD(int, FormatVersion, "")
	CREFLECT_STRUCT_FIELD(int, Resolution, "")
	CREFLECT_STRUCT_FIELD(int, SamplesPerTexel, "")
	CREFLECT_STRUCT_FIELD(int, Bounces, "")
	CREFLECT_STRUCT_FIELD(Vector3, Position, "Noedit")
	CREFLECT_STRUCT_FIELD(Vector3, Extent, "Noedit")
	CREFLECT_STRUCT_FIELD(TextureStoredPixels, Radiance, "Noedit")
CREFLECT_STRUCT_END()

CREFLECT_STRUCT_BEGIN(HLevelLightingData, "")
	CREFLECT_STRUCT_FIELD_ARRAY(HIrradianceVolumeData, IrradianceVolumes, "")
	CREFLECT_STRUCT_FIELD_ARRAY(HReflectionCaptureData, ReflectionCaptures, "")
CREFLECT_STRUCT_END()
