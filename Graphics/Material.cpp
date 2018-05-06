/************************************************
*                 Material.cpp                  *
*************************************************
*          This file is a part of:              *
*               COLUMBUS ENGINE                 *
*************************************************
*                Nika(Columbus) Red             *
*                   20.07.2017                  *
*************************************************/

#include <Graphics/Material.h>

using nlohmann::json;

namespace Columbus
{

	//////////////////////////////////////////////////////////////////////////////
	Material::Material() :
		mReflectionPower(0.2),
		mTexture(nullptr),
		mSpecMap(nullptr),
		mNormMap(nullptr),
		mDiscard(false),
		mLighting(false),
		mEnvReflection(nullptr),
		mShininess(32)
	{
		
	}
	//////////////////////////////////////////////////////////////////////////////
	Material::Material(std::string aFile)
	{
		loadFromXML(aFile);
	}
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	void Material::setColor(const Vector4 aColor)
	{
		mColor = static_cast<Vector4>(aColor);
	}
	//////////////////////////////////////////////////////////////////////////////
	void Material::setAmbient(const Vector3 aAmbient)
	{
		mAmbient = static_cast<Vector3>(aAmbient);
	}
	//////////////////////////////////////////////////////////////////////////////
	void Material::setDiffuse(const Vector3 aDiffuse)
	{
		mDiffuse = static_cast<Vector3>(aDiffuse);
	}
	//////////////////////////////////////////////////////////////////////////////
	void Material::setSpecular(const Vector3 aSpecular)
	{
		mSpecular = static_cast<Vector3>(aSpecular);
	}
	//////////////////////////////////////////////////////////////////////////////
	void Material::setShininess(const float aShininess)
	{
		mShininess = static_cast<float>(aShininess);
	}
	//////////////////////////////////////////////////////////////////////////////
	void Material::setTexture(const Texture* aTexture)
	{
		mTexture = const_cast<Texture*>(aTexture);
	}
	//////////////////////////////////////////////////////////////////////////////
	void Material::setSpecMap(const Texture* aSpecMap)
	{
		mSpecMap = const_cast<Texture*>(aSpecMap);
	}
	//////////////////////////////////////////////////////////////////////////////
	void Material::setNormMap(const Texture* aNormMap)
	{
		mNormMap = const_cast<Texture*>(aNormMap);
	}
	
	void Material::SetShader(ShaderProgram* InShader)
	{
		ShaderProg = InShader;
	}

	//////////////////////////////////////////////////////////////////////////////
	void Material::setReflection(const Cubemap* aReflecction)
	{
		mEnvReflection = const_cast<Cubemap*>(aReflecction);
	}
	//////////////////////////////////////////////////////////////////////////////
	void Material::setReflectionPower(const float aPower)
	{
		mReflectionPower = static_cast<float>(aPower);
	}
	//////////////////////////////////////////////////////////////////////////////
	void Material::setDiscard(const bool aDiscard)
	{
		mDiscard = static_cast<bool>(aDiscard);
	}
	//////////////////////////////////////////////////////////////////////////////
	void Material::setLighting(const bool aLighting)
	{
		mLighting = static_cast<bool>(aLighting);
	}
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	Vector4 Material::getColor() const
	{
		return mColor;
	}
	//////////////////////////////////////////////////////////////////////////////
	Vector3 Material::getAmbient() const
	{
		return mAmbient;
	}
	//////////////////////////////////////////////////////////////////////////////
	Vector3 Material::getDiffuse() const
	{
		return mDiffuse;
	}
	//////////////////////////////////////////////////////////////////////////////
	Vector3 Material::getSpecular() const
	{
		return mSpecular;
	}
	//////////////////////////////////////////////////////////////////////////////
	float Material::getShininess() const
	{
		return mShininess;
	}
	//////////////////////////////////////////////////////////////////////////////
	Texture* Material::getTexture() const
	{
		return mTexture;
	}
	//////////////////////////////////////////////////////////////////////////////
	Texture* Material::getSpecMap() const
	{
		return mSpecMap;
	}
	//////////////////////////////////////////////////////////////////////////////
	Texture* Material::getNormMap() const
	{
		return mNormMap;
	}

	ShaderProgram* Material::GetShader() const
	{
		return ShaderProg;
	}

	Cubemap* Material::getReflection() const
	{
		return mEnvReflection;
	}
	//////////////////////////////////////////////////////////////////////////////
	float Material::getReflectionPower() const
	{
		return mReflectionPower;
	}
	//////////////////////////////////////////////////////////////////////////////
	bool Material::getDiscard() const
	{
		return mDiscard;
	}
	//////////////////////////////////////////////////////////////////////////////
	bool Material::getLighting() const
	{
		return mLighting;
	}
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	int Material::getTextureID() const
	{
		return mTextureID;
	}
	//////////////////////////////////////////////////////////////////////////////
	int Material::getSpecMapID() const
	{
		return mSpecMapID;
	}
	//////////////////////////////////////////////////////////////////////////////
	int Material::getNormMapID() const
	{
		return mNormMapID;
	}
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	bool Material::saveToXML(std::string aFile) const
	{
		Serializer::SerializerXML Serializer;

		if (!Serializer.Write(aFile, "Material"))
		{ Log::error("Can't save Material: " + aFile); return false; }

		if (!Serializer.SetVector4("Color", mColor, {"R", "G", "B", "A"}))
		{ Log::error("Can't save Material color: " + aFile); return false; }

		if (!Serializer.SetVector3("Ambient", mAmbient, {"R", "G", "B"}))
		{ Log::error("Can't save Material ambient: " + aFile); return false; }

		if (!Serializer.SetVector3("Diffuse", mDiffuse, {"R", "G", "B"}))
		{ Log::error("Can't save Material diffuse: " + aFile); return false; }

		if (!Serializer.SetVector3("Specular", mSpecular, {"R", "G", "B"}))
		{ Log::error("Can't save Material specular: " + aFile); return false; }

		if (!Serializer.SetFloat("Shininess", mShininess))
		{ Log::error("Can't save Material shininess: " + aFile); return false; }

		if (!Serializer.SetFloat("ReflectionPower", mReflectionPower))
		{ Log::error("Can't save Material reflection power: " + aFile); return false; }

		if (!Serializer.SetBool("Discard", mDiscard))
		{ Log::error("Can't save Material discard: " + aFile); return false; }

		if (!Serializer.SetBool("Lighting", mLighting))
		{ Log::error("Can't save Material lighting: " + aFile); return false; }

		if (!Serializer.Save())
		{ Log::error("Can't save Material: " + aFile); return false; }

		Log::success("Material saved: " + aFile);

		return true;
	}
	//////////////////////////////////////////////////////////////////////////////
	bool Material::saveToJSON(std::string aFile) const
	{
		json j;
		
		j["Material"]["Color"] = { mColor.x, mColor.y, mColor.z, mColor.w };
		j["Material"]["Ambient"] = { mAmbient.X, mAmbient.Y, mAmbient.Z };
		j["Material"]["Diffuse"] = { mDiffuse.X, mDiffuse.Y, mDiffuse.Z };
		j["Material"]["Specular"] = { mSpecular.X, mSpecular.Y, mSpecular.Z };
		j["Material"]["Shininess"] = mShininess;
		j["Material"]["ReflectionPower"] = mReflectionPower;
		j["Material"]["Discard"] = mDiscard;
		j["Material"]["Lighting"] = mLighting;
	
		std::ofstream o(aFile);
		
		if (o.is_open() == false)
		{
			Log::error("Can't save Material: " + aFile);
			return false;
		}

		o << std::setw(4) << j << std::endl;
		o.close();

		Log::success("Material saved: " + aFile);

		return true;
	}
	//////////////////////////////////////////////////////////////////////////////
	bool Material::loadFromXML(std::string aFile)
	{
		Serializer::SerializerXML Serializer;

		if (!Serializer.Read(aFile, "Material"))
		{ Log::error("Can't load Material: " + aFile); return false; }

		if (!Serializer.GetVector4("Color", mColor, {"R", "G", "B", "A"}))
		{ Log::error("Can't load Material color: " + aFile); return false; }

		if (!Serializer.GetVector3("Ambient", mAmbient, {"R", "G", "B"}))
		{ Log::error("Can't load Material ambient: " + aFile); return false; }

		if (!Serializer.GetVector3("Diffuse", mDiffuse, {"R", "G", "B"}))
		{ Log::error("Can't load Material diffuse: " + aFile); return false; }

		if (!Serializer.GetVector3("Specular", mSpecular, {"R", "G", "B"}))
		{ Log::error("Can't load Material specular: " + aFile); return false; }

		if (!Serializer.GetFloat("Shininess", mShininess))
		{ Log::error("Can't load Material shininess: " + aFile); return false; }

		if (!Serializer.GetFloat("ReflectionPower", mReflectionPower))
		{ Log::error("Can't load Material reflection power: " + aFile); return false; }

		if (!Serializer.GetBool("Discard", mDiscard))
		{ Log::error("Can't load Material discard: " + aFile); return false; }

		if (!Serializer.GetBool("Lighting", mLighting))
		{ Log::error("Can't load Material lighting: " + aFile); return false; }

		std::string diffuseMapPath = "None";
		std::string specularMapPath = "None";
		std::string normalMapPath = "None";

		Serializer.GetSubString({"Textures", "Diffuse"}, diffuseMapPath);
		Serializer.GetSubString({"Textures", "Specular"}, specularMapPath);
		Serializer.GetSubString({"Textures", "Normal"}, normalMapPath);

		if (diffuseMapPath != "None")
			mTextureID = std::atoi(diffuseMapPath.c_str());

		if (specularMapPath != "None")
			mSpecMapID = std::atoi(specularMapPath.c_str());

		if (normalMapPath != "None")
			mNormMapID = std::atoi(normalMapPath.c_str());

		Log::success("Material loaded: " + aFile);

		return true;
	}
	//////////////////////////////////////////////////////////////////////////////
	bool Material::loadFromJSON(std::string aFile)
	{
		std::ifstream i(aFile);
		if (i.is_open() == false)
		{
			Log::error("Can't load Material: " + aFile);
			return false;
		}
		json j;
		i >> j;

		mColor.x = j["Material"]["Color"][0];
		mColor.y = j["Material"]["Color"][1];
		mColor.z = j["Material"]["Color"][2];
		mColor.w = j["Material"]["Color"][3];

		mAmbient.X = j["Material"]["Ambient"][0];
		mAmbient.Y = j["Material"]["Ambient"][1];
		mAmbient.Z = j["Material"]["Ambient"][2];

		mDiffuse.X = j["Material"]["Diffuse"][0];
		mDiffuse.Y = j["Material"]["Diffuse"][1];
		mDiffuse.Z = j["Material"]["Diffuse"][2];

		mSpecular.X = j["Material"]["Specular"][0];
		mSpecular.Y = j["Material"]["Specular"][1];
		mSpecular.Z = j["Material"]["Specular"][2];

		mShininess = j["Material"]["Shininess"];
		mReflectionPower = j["Material"]["ReflectionPower"];
		mDiscard = j["Material"]["Discard"];
		mLighting = j["Material"]["Lighting"];

		i.close();
		
		Log::success("Material loaded: " + aFile);

		return true;
	}
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	Material::~Material()
	{

	}

}
