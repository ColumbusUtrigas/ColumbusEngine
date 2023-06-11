#pragma once

#include <Math/Vector3.h>

namespace Columbus
{

	struct IrradianceVolume
	{
		alignas(16) Vector3 Position;
		alignas(16) Vector3 Extent;
		alignas(16) iVector3 ProbesCount;

		int GetTotalProbes() const
		{
			return ProbesCount.X * ProbesCount.Y * ProbesCount.Z;
		}
	};

}
