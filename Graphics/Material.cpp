#include <Graphics/Material.h>

namespace Columbus
{

	Material::Material() :
		mLighting(false) {}
	
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
	
	void Material::setLighting(const bool aLighting)
	{
		mLighting = static_cast<bool>(aLighting);
	}

	ShaderProgram* Material::GetShader() const
	{
		return ShaderProg;
	}
	
	bool Material::getLighting() const
	{
		return mLighting;
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
	
	bool Material::saveToXML(std::string aFile) const
	{
		Serializer::SerializerXML Serializer;

		/*if (!Serializer.Write(aFile, "Material"))
		{ Log::error("Can't save Material: " + aFile); return false; }

		if (!Serializer.SetVector4("Color", mColor, {"R", "G", "B", "A"}))
		{ Log::error("Can't save Material color: " + aFile); return false; }

		if (!Serializer.SetVector3("Ambient", mAmbient, {"R", "G", "B"}))
		{ Log::error("Can't save Material ambient: " + aFile); return false; }

		if (!Serializer.SetVector3("Diffuse", mDiffuse, {"R", "G", "B"}))
		{ Log::error("Can't save Material diffuse: " + aFile); return false; }

		if (!Serializer.SetVector3("Specular", mSpecular, {"R", "G", "B"}))
		{ Log::error("Can't save Material specular: " + aFile); return false; }

		if (!Serializer.SetFloat("ReflectionPower", mReflectionPower))
		{ Log::error("Can't save Material reflection power: " + aFile); return false; }

		if (!Serializer.SetBool("Lighting", mLighting))
		{ Log::error("Can't save Material lighting: " + aFile); return false; }

		if (!Serializer.Save())
		{ Log::error("Can't save Material: " + aFile); return false; }*/

		Log::success("Material saved: " + aFile);

		return true;
	}
	
	bool Material::loadFromXML(std::string aFile)
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

		if (!Serializer.GetFloat("ReflectionPower", ReflectionPower))
		{ Log::error("Can't load Material reflection power: " + aFile); return false; }

		if (!Serializer.GetFloat("EmissionStrength", EmissionStrength))
		{ Log::error("Can't load Material emission strength: " + aFile); return false; }

		if (!Serializer.GetFloat("DetailNormalStrength", DetailNormalStrength))
		{ Log::error("Can't load Material detail normal strength: " + aFile); return false; }

		if (!Serializer.GetFloat("Rim", Rim))
		{ Log::error("Can't load Material Rim: " + aFile); return false; }

		if (!Serializer.GetFloat("RimPower", RimPower))
		{ Log::error("Can't load Material Rim power: " + aFile); return false; }

		if (!Serializer.GetFloat("RimBias", RimBias))
		{ Log::error("Can't load Material Rim bias: " + aFile); return false; }

		if (!Serializer.GetVector3("RimColor", RimColor, { "R", "G", "B" }))
		{ Log::error("Can't load Material Rim color: " + aFile); return false; }

		if (!Serializer.GetBool("Lighting", mLighting))
		{ Log::error("Can't load Material lighting: " + aFile); return false; }

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

	bool Material::operator==(Material Other) const
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
		        ReflectionPower == Other.ReflectionPower &&
		        EmissionStrength == Other.EmissionStrength &&
		        Rim == Other.Rim &&
		        RimPower == Other.RimPower &&
		        RimBias == Other.RimBias &&
		        RimColor == Other.RimColor &&
		        mLighting == Other.mLighting);
	}

	bool Material::operator!=(Material Other) const
	{
		return !(*this == Other);
	}

	Material::~Material()
	{

	}

}




