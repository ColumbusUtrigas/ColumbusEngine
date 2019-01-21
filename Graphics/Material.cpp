#include <Graphics/Material.h>
#include <System/Log.h>
#include <System/Serializer.h>

namespace Columbus
{

	Material::Material() {}
	
	Material::Material(const char* FileName)
	{
		LoadFromXML(FileName);
	}

	bool Material::Prepare()
	{
		if (ShaderProg != nullptr)
		{
			if (!ShaderProg->IsCompiled())
			{
				ShaderProg->Compile();
			}

			ShaderProg->Bind();

			return true;
		}

		return false;
	}
	
	void Material::SetShader(ShaderProgram* InShader)
	{
		ShaderProg = InShader;
	}

	ShaderProgram* Material::GetShader() const
	{
		return ShaderProg;
	}
	
	int Material::getTextureID() const
	{
		return mTextureID;
	}

	int Material::getNormMapID() const
	{
		return mNormMapID;
	}

	int Material::GetRoughnessMapID() const
	{
		return RoughnessMapID;
	}

	int Material::GetMetallicMapID() const
	{
		return MetallicMapID;
	}

	int Material::GetOcclusionMapID() const
	{
		return OcclusionMapID;
	}

	int Material::GetEmissionMapID() const
	{
		return EmissionMapID;
	}

	int Material::GetDetailDiffuseMapID() const
	{
		return DetailDiffuseMapID;
	}

	int Material::GetDetailNormalMapID() const
	{
		return DetailNormalMapID;
	}
	
	bool Material::LoadFromXML(const char* FileName)
	{
		Serializer::SerializerXML Serializer;

		if (!Serializer.Read(FileName, "Material"))
		{ Log::Error("Can't load Material: %s", FileName); return false; }

		std::string MaterialCulling;

		if (!Serializer.GetString("Culling", MaterialCulling))
		{ Log::Error("Can't load Material culling: %s", FileName); return false; }

		if (!Serializer.GetBool("DepthWriting", DepthWriting))
		{ Log::Error("Can't load Material depth writing: %s", FileName); return false; }

		if (!Serializer.GetBool("Transparent", Transparent))
		{ Log::Error("Can't load Material transparent: %s", FileName); return false; }

		if (!Serializer.GetVector2("Tiling", Tiling, { "X", "Y"}))
		{ Log::Error("Can't load Material tiling: %s", FileName); return false; }

		if (!Serializer.GetVector2("DetailTiling", DetailTiling, { "X", "Y"}))
		{ Log::Error("Can't load Material detail tiling: %s", FileName); return false; }

		if (!Serializer.GetVector4("Color", Color, { "R", "G", "B", "A" }))
		{ Log::Error("Can't load Material color: %s", FileName); return false; }

		if (!Serializer.GetFloat("Roughness", Roughness))
		{ Log::Error("Can't load Material roughness: %s", FileName); return false; }

		if (!Serializer.GetFloat("Metallic", Metallic))
		{ Log::Error("Can't load Material metallic: %s", FileName); return false; }

		if (!Serializer.GetFloat("EmissionStrength", EmissionStrength))
		{ Log::Error("Can't load Material emission strength: %s", FileName); return false; }

		std::string diffuseMapPath = "None";
		std::string normalMapPath = "None";
		std::string RoughnessMapPath = "None";
		std::string MetallicMapPath = "None";
		std::string OcclusionMapPath = "None";
		std::string EmissionMapPath = "None";
		std::string DetailDiffuseMapPath = "None";
		std::string DetailNormalMapPath = "None";

		Serializer.GetSubString({"Textures", "Diffuse"}, diffuseMapPath);
		Serializer.GetSubString({"Textures", "Normal"}, normalMapPath);
		Serializer.GetSubString({"Textures", "Roughness"}, RoughnessMapPath);
		Serializer.GetSubString({"Textures", "Metallic"}, MetallicMapPath);
		Serializer.GetSubString({"Textures", "Occlusion"}, OcclusionMapPath);
		Serializer.GetSubString({"Textures", "Emission"}, EmissionMapPath);
		Serializer.GetSubString({"Textures", "DetailDiffuse"}, DetailDiffuseMapPath);
		Serializer.GetSubString({"Textures", "DetailNormal"}, DetailNormalMapPath);

		if (diffuseMapPath != "None")
		{
			mTextureID = std::atoi(diffuseMapPath.c_str());
		}

		if (normalMapPath != "None")
		{
			mNormMapID = std::atoi(normalMapPath.c_str());
		}

		if (RoughnessMapPath != "None")
		{
			RoughnessMapID = std::atoi(RoughnessMapPath.c_str());
		}

		if (MetallicMapPath != "None")
		{
			MetallicMapID = std::atoi(MetallicMapPath.c_str());
		}

		if (OcclusionMapPath != "None")
		{
			OcclusionMapID = std::atoi(OcclusionMapPath.c_str());
		}

		if (EmissionMapPath != "None")
		{
			EmissionMapID = std::atoi(EmissionMapPath.c_str());
		}

		if (DetailDiffuseMapPath != "None")
		{
			DetailDiffuseMapID = std::atoi(DetailDiffuseMapPath.c_str());
		}

		if (DetailNormalMapPath != "None")
		{
			DetailNormalMapID = std::atoi(DetailNormalMapPath.c_str());
		}

		if (MaterialCulling == "No")
		{
			Culling = Cull::No;
		}
		else if (MaterialCulling == "Front")
		{
			Culling = Cull::Front;
		}
		else if (MaterialCulling == "Back")
		{
			Culling = Cull::Back;
		}
		else if (MaterialCulling == "FrontAndBack")
		{
			Culling = Cull::FrontAndBack;
		}

		Log::Success("Material loaded: %s", FileName);

		return true;
	}

	bool Material::LoadFromJSON(const char* FileName)
	{
		return false;
	}

	bool Material::Load(const char* FileName)
	{
		if (strcmp(strrchr(FileName, '.'), ".cxmat") == 0)
		{
			return LoadFromXML(FileName);
		}

		if (strcmp(strrchr(FileName, '.'), ".cjmat") == 0)
		{
			return LoadFromJSON(FileName);
		}

		return false;
	}

	bool Material::operator==(const Material& Other) const
	{
		return (Color == Other.Color &&
		        DiffuseTexture == Other.DiffuseTexture &&
		        NormalTexture == Other.NormalTexture &&
		        RoughnessTexture == Other.RoughnessTexture &&
		        MetallicTexture == Other.MetallicTexture &&
		        OcclusionMap == Other.OcclusionMap &&
		        EmissionMap == Other.EmissionMap &&
		        DetailDiffuseMap == Other.DetailDiffuseMap &&
		        DetailNormalMap == Other.DetailNormalMap &&
		        Reflection == Other.Reflection &&
		        ShaderProg == Other.ShaderProg &&
		        Roughness == Other.Roughness &&
		        Metallic == Other.Metallic &&
		        EmissionStrength == Other.EmissionStrength &&
		        Lighting == Other.Lighting);
	}

	bool Material::operator!=(const Material& Other) const
	{
		return !(*this == Other);
	}

	Material::~Material()
	{

	}

}




