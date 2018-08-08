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
	
	int Material::getSpecMapID() const
	{
		return mSpecMapID;
	}

	int Material::getNormMapID() const
	{
		return mNormMapID;
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

		if (!Serializer.GetVector2("Tiling", Tiling, { "X", "Y"}))
		{ Log::error("Can't load Material tiling: " + aFile); return false; }

		if (!Serializer.GetVector2("DetailTiling", DetailTiling, { "X", "Y"}))
		{ Log::error("Can't load Material detail tiling: " + aFile); return false; }

		if (!Serializer.GetVector4("Color", Color, { "R", "G", "B", "A" }))
		{ Log::error("Can't load Material color: " + aFile); return false; }

		if (!Serializer.GetVector3("Ambient", AmbientColor, { "R", "G", "B" }))
		{ Log::error("Can't load Material ambient color: " + aFile); return false; }

		if (!Serializer.GetVector3("Diffuse", DiffuseColor, { "R", "G", "B" }))
		{ Log::error("Can't load Material diffuse color: " + aFile); return false; }

		if (!Serializer.GetVector3("Specular", SpecularColor, { "R", "G", "B" }))
		{ Log::error("Can't load Material specular color: " + aFile); return false; }

		if (!Serializer.GetFloat("ReflectionPower", ReflectionPower))
		{ Log::error("Can't load Material reflection power: " + aFile); return false; }

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
		std::string specularMapPath = "None";
		std::string normalMapPath = "None";
		std::string DetailDiffuseMapPath = "None";
		std::string DetailNormalMapPath = "None";

		Serializer.GetSubString({"Textures", "Diffuse"}, diffuseMapPath);
		Serializer.GetSubString({"Textures", "Specular"}, specularMapPath);
		Serializer.GetSubString({"Textures", "Normal"}, normalMapPath);
		Serializer.GetSubString({"Textures", "DetailDiffuse"}, DetailDiffuseMapPath);
		Serializer.GetSubString({"Textures", "DetailNormal"}, DetailNormalMapPath);

		if (diffuseMapPath != "None")
		{
			mTextureID = std::atoi(diffuseMapPath.c_str());
		}

		if (specularMapPath != "None")
		{
			mSpecMapID = std::atoi(specularMapPath.c_str());
		}

		if (normalMapPath != "None")
		{
			mNormMapID = std::atoi(normalMapPath.c_str());
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

	bool Material::operator==(Material Other)
	{
		return (Color == Other.Color &&
		        AmbientColor == Other.AmbientColor &&
		        DiffuseColor == Other.DiffuseColor &&
		        SpecularColor == Other.SpecularColor &&
		        DiffuseTexture == Other.DiffuseTexture &&
		        SpecularTexture == Other.SpecularTexture &&
		        NormalTexture == Other.NormalTexture &&
		        ShaderProg == Other.ShaderProg &&
		        ReflectionPower == Other.ReflectionPower &&
		        Rim == Other.Rim &&
		        RimPower == Other.RimPower &&
		        RimBias == Other.RimBias &&
		        RimColor == Other.RimColor &&
		        mLighting == Other.mLighting &&
		        mTextureID == Other.mTextureID &&
		        mSpecMapID == Other.mSpecMapID &&
		        mNormMapID == Other.mNormMapID);
	}

	bool Material::operator!=(Material Other)
	{
		return !(*this == Other);
	}

	Material::~Material()
	{

	}

}




