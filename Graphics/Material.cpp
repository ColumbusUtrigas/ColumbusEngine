#include <Graphics/Material.h>

namespace Columbus
{

	Material::Material() {}
	
	Material::Material(std::string aFile)
	{
		loadFromXML(aFile);
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
	
	bool Material::loadFromXML(const std::string& aFile)
	{
		Serializer::SerializerXML Serializer;

		if (!Serializer.Read(aFile, "Material"))
		{ Log::error("Can't load Material: " + aFile); return false; }

		std::string MaterialCulling;

		if (!Serializer.GetString("Culling", MaterialCulling))
		{ Log::error("Can't load Material culling: " + aFile); return false; }

		if (!Serializer.GetBool("DepthWriting", DepthWriting))
		{ Log::error("Can't load Material depth writing: " + aFile); return false; }

		if (!Serializer.GetBool("Transparent", Transparent))
		{ Log::error("Can't load Material transparent: " + aFile); return false; }

		if (!Serializer.GetVector2("Tiling", Tiling, { "X", "Y"}))
		{ Log::error("Can't load Material tiling: " + aFile); return false; }

		if (!Serializer.GetVector2("DetailTiling", DetailTiling, { "X", "Y"}))
		{ Log::error("Can't load Material detail tiling: " + aFile); return false; }

		if (!Serializer.GetVector4("Color", Color, { "R", "G", "B", "A" }))
		{ Log::error("Can't load Material color: " + aFile); return false; }

		if (!Serializer.GetVector3("Ambient", AmbientColor, { "R", "G", "B" }))
		{ Log::error("Can't load Material ambient color: " + aFile); return false; }

		if (!Serializer.GetFloat("Roughness", Roughness))
		{ Log::error("Can't load Material roughness: " + aFile); return false; }

		if (!Serializer.GetFloat("Metallic", Metallic))
		{ Log::error("Can't load Material metallic: " + aFile); return false; }

		if (!Serializer.GetFloat("EmissionStrength", EmissionStrength))
		{ Log::error("Can't load Material emission strength: " + aFile); return false; }

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

		Log::success("Material loaded: " + aFile);

		return true;
	}

	bool Material::operator==(const Material& Other) const
	{
		return (Color == Other.Color &&
		        AmbientColor == Other.AmbientColor &&
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




