#pragma once

#include <Math/Vector4.h>
#include <Core/Asset.h>
#include <Core/StableSparseArray.h>
#include <Core/Reflection.h>
#include <Graphics/Core/Texture.h>

namespace Columbus
{

	struct Material
	{
		Vector4 AlbedoFactor;
		Vector4 EmissiveFactor;

		// textures
		AssetRef<Texture2> Albedo;
		AssetRef<Texture2> Normal;
		AssetRef<Texture2> Orm; // r - occlusion, g - roughness, b - metallic, a - unused
		AssetRef<Texture2> Emissive;

		// texture GPU scene proxies
		TStableSparseArray<Texture2*>::Handle AlbedoId;
		TStableSparseArray<Texture2*>::Handle NormalId;
		TStableSparseArray<Texture2*>::Handle OrmId;
		TStableSparseArray<Texture2*>::Handle EmissiveId;

		// used when no Orm
		float Roughness = 1;
		float Metallic = 0;

		// GPUScene ref
		TStableSparseArray<Material>::Handle StableId;
	};

}

CREFLECT_DECLARE_STRUCT(Columbus::Material, 1, "F83F16E6-9E7E-4A8A-B2DC-C11216858BE7");
