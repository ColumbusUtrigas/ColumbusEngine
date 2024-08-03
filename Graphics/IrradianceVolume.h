#pragma once

#include <Math/Vector3.h>
#include "Core/Buffer.h"

namespace Columbus
{

	struct IrradianceVolume
	{
		alignas(16) Vector3 Position;
		alignas(16) Vector3 Extent;
		alignas(16) iVector3 ProbesCount;

		Vector3 TestPoint;

		Buffer* ProbesBuffer = nullptr;

		int GetTotalProbes() const
		{
			return ProbesCount.X * ProbesCount.Y * ProbesCount.Z;
		}
	};

}
