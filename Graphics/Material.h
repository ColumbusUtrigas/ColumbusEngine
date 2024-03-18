#pragma once

#include <Math/Vector4.h>

namespace Columbus
{

	struct Material
	{
		Vector4 AlbedoFactor;
		Vector4 EmissiveFactor;

		// textures
		int AlbedoId = -1;
		int NormalId = -1;
		int OrmId = -1; // r - occlusion, g - roughness, b - metallic, a - unused
		int EmissiveId = -1;

		// used when OrmId == -1
		float Roughness = 1;
		float Metallic = 0;
	};

}
