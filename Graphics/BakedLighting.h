#pragma once

#include "Core/Asset.h"
#include "Core/Blob.h"
#include "Core/SmartPointer.h"
#include "Core/ThingRef.h"
#include "Core/Types.h"
#include "Graphics/Core/Texture.h"
#include "Math/Vector3.h"

#include <vector>

namespace Columbus
{
	struct AIrradianceVolume;
	struct AReflectionCaptureBox;
	struct EngineWorld;
	struct GPUIrradianceVolume;
	struct GPUReflectionCapture;
	class DeviceVulkan;

	static constexpr int IrradianceVolumeBakeFormatVersion = 1;
	static constexpr int ReflectionCaptureBakeFormatVersion = 1;

	///////////////////////////////////////////////////////////////////////////////////////////////
	// Serialized baked lighting (.clight)
	//
	// Only data that affects the baked payload lives here. Sampling-time parameters
	// (intensity, priority, blend distance, normal bias) belong to the authoring things
	// and flow into GPUScene directly, so they never require a rebake.

	struct HIrradianceVolumeData
	{
		ThingRef<AIrradianceVolume> Owner;

		int FormatVersion = IrradianceVolumeBakeFormatVersion;
		int ProbeCountX = 0;
		int ProbeCountY = 0;
		int ProbeCountZ = 0;
		int ProbeStride = 0;
		int RaysPerProbe = 0;
		int Bounces = 0;

		// transform the probes were baked with
		Vector3 Position;
		Vector3 Extent;

		Blob Probes;
	};

	struct HReflectionCaptureData
	{
		ThingRef<AReflectionCaptureBox> Owner;

		int FormatVersion = ReflectionCaptureBakeFormatVersion;
		int Resolution = 0;
		int SamplesPerTexel = 0;
		int Bounces = 0;

		// transform the capture was baked with
		Vector3 Position;
		Vector3 Extent;

		// BC6H cube with a full mip chain
		TextureStoredPixels Radiance;
	};

	struct HLevelLightingData
	{
		std::vector<HIrradianceVolumeData>  IrradianceVolumes;
		std::vector<HReflectionCaptureData> ReflectionCaptures;
	};

	///////////////////////////////////////////////////////////////////////////////////////////////
	// Baked lighting operations

	namespace BakedLighting
	{
		int ClampReflectionCaptureResolution(int Resolution);

		// validates a reflection capture payload (BC6H cube, square power-of-two, full mip
		// chain, byte size); logs and returns false on failure
		bool ValidateReflectionCaptureData(const HReflectionCaptureData& Entry);

		// reads the baked probe buffer into Data.Probes; false on failure
		bool ReadbackIrradianceProbes(SPtr<DeviceVulkan> Device, const GPUIrradianceVolume& Volume, HIrradianceVolumeData& Data);

		// reads the baked HDR cubemap (all faces and mips), compresses it to BC6H and stores
		// the result into Data.Radiance; false on failure
		bool ReadbackReflectionRadiance(SPtr<DeviceVulkan> Device, const GPUReflectionCapture& Capture, HReflectionCaptureData& Data);

		// validates entries, matches them to live things by owner guid and uploads probe
		// buffers / radiance cubemaps into GPUScene. invalid or unmatched entries are skipped.
		void Apply(EngineWorld& World, const HLevelLightingData& LightingData);
	}
}

CREFLECT_DECLARE_STRUCT(Columbus::HIrradianceVolumeData, 1, "F23A1411-5B9C-4BFC-862A-5EA440AA5B65");
CREFLECT_DECLARE_STRUCT(Columbus::HReflectionCaptureData, 1, "C1E81966-17E1-4BA6-B745-732BFDDE8F9D");
CREFLECT_DECLARE_STRUCT(Columbus::HLevelLightingData, 1, "37C44FA7-4362-46B6-861C-8D3E36377314");
