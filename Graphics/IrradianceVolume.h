#pragma once

#include <Math/Vector3.h>
#include <Math/Vector2.h>
#include <Math/Vector4.h>
#include <Core/Blob.h>
#include <Core/ThingRef.h>
#include <string>
#include <vector>
#include "Core/Types.h"
#include "Core/Buffer.h"
#include "Profiling/Profiling.h"

DECLARE_MEMORY_PROFILING_COUNTER(MemoryCounter_SceneIrradianceProbes);

namespace Columbus
{
	struct AIrradianceVolume;

	static constexpr int IrradianceVolumeBakeFormatVersion = 1;

	struct GPUIrradianceProbe
	{
		alignas(16) Vector4 PositionState;
		alignas(16) Vector4 Irradiance[6];
		Vector2 Distance[64];
	};

	struct HIrradianceVolumeBakeBuffer
	{
		ThingRef<AIrradianceVolume> Owner;
		int FormatVersion = IrradianceVolumeBakeFormatVersion;
		int ProbeStride = sizeof(GPUIrradianceProbe);
		int ProbeCount = 0;
		Blob ProbeData;
	};

	struct HLevelLightingData
	{
		std::vector<HIrradianceVolumeBakeBuffer> IrradianceVolumes;
	};

	struct IrradianceVolume
	{
		alignas(16) Vector3 Position;
		alignas(16) Vector3 Extent;
		alignas(16) iVector3 ProbesCount;
		float Intensity = 1.0f;
		float NormalBias = 0.2f;
		float BlendDistance = 0.0f; // 0: derive from probe spacing.
		float Priority = 0.0f;      // 0: derive from probe density.
		bool bVisualiseProbes = true;

		Vector3 TestPoint;
		u64 OwnerGuid = 0;

		Buffer* ProbesBuffer = nullptr;
		u64 ProbesBufferBytes = 0;

		int GetTotalProbes() const
		{
			return ProbesCount.X * ProbesCount.Y * ProbesCount.Z;
		}
	};

}

CREFLECT_DECLARE_STRUCT(Columbus::HIrradianceVolumeBakeBuffer, 1, "D9F97DE5-9232-4BC6-B530-4FB52E915701");
CREFLECT_DECLARE_STRUCT(Columbus::HLevelLightingData, 1, "37C44FA7-4362-46B6-861C-8D3E36377314");
