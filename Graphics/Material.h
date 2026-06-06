#pragma once

#include <Math/Vector4.h>
#include <Core/Asset.h>
#include <Core/StableSparseArray.h>
#include <Core/Reflection.h>
#include <Graphics/Core/Texture.h>

namespace Columbus
{
	enum class MaterialShadingMode
	{
		Opaque = 0,
		Transparent = 1,
		Refractive = 2,
		Masked = 3,
	};

	struct Material
	{
		Vector4 AlbedoFactor;
		Vector4 EmissiveFactor;
		MaterialShadingMode ShadingMode = MaterialShadingMode::Opaque;
		float AlphaCutoff = 0.5f;

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

CREFLECT_DECLARE_ENUM(Columbus::MaterialShadingMode, "7CD59391-C096-4B81-8282-A88D5A2E3C0B");
CREFLECT_DECLARE_STRUCT(Columbus::Material, 3, "F83F16E6-9E7E-4A8A-B2DC-C11216858BE7");
