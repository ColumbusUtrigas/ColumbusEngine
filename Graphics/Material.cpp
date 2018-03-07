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
	C_Material::C_Material() :
		mReflectionPower(0.2),
		mTexture(nullptr),
		mSpecMap(nullptr),
		mNormMap(nullptr),
		mShader(nullptr),
		mDiscard(false),
		mLighting(false),
		mEnvReflection(nullptr),
		mShininess(32)
	{
		
	}
	//////////////////////////////////////////////////////////////////////////////
	C_Material::C_Material(std::string aFile)
	{
		loadFromXML(aFile);
	}
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	void C_Material::setColor(const Vector4 aColor)
	{
		mColor = static_cast<Vector4>(aColor);
	}
	//////////////////////////////////////////////////////////////////////////////
	void C_Material::setAmbient(const Vector3 aAmbient)
	{
		mAmbient = static_cast<Vector3>(aAmbient);
	}
	//////////////////////////////////////////////////////////////////////////////
	void C_Material::setDiffuse(const Vector3 aDiffuse)
	{
		mDiffuse = static_cast<Vector3>(aDiffuse);
	}
	//////////////////////////////////////////////////////////////////////////////
	void C_Material::setSpecular(const Vector3 aSpecular)
	{
		mSpecular = static_cast<Vector3>(aSpecular);
	}
	//////////////////////////////////////////////////////////////////////////////
	void C_Material::setShininess(const float aShininess)
	{
		mShininess = static_cast<float>(aShininess);
	}
	//////////////////////////////////////////////////////////////////////////////
	void C_Material::setTexture(const Texture* aTexture)
	{
		mTexture = const_cast<Texture*>(aTexture);
	}
	//////////////////////////////////////////////////////////////////////////////
	void C_Material::setSpecMap(const Texture* aSpecMap)
	{
		mSpecMap = const_cast<Texture*>(aSpecMap);
	}
	//////////////////////////////////////////////////////////////////////////////
	void C_Material::setNormMap(const Texture* aNormMap)
	{
		mNormMap = const_cast<Texture*>(aNormMap);
	}
	//////////////////////////////////////////////////////////////////////////////
	void C_Material::setShader(const Shader* aShader)
	{
		mShader = const_cast<Shader*>(aShader);
	}
	//////////////////////////////////////////////////////////////////////////////
	void C_Material::setReflection(const Cubemap* aReflecction)
	{
		mEnvReflection = const_cast<Cubemap*>(aReflecction);
	}
	//////////////////////////////////////////////////////////////////////////////
	void C_Material::setReflectionPower(const float aPower)
	{
		mReflectionPower = static_cast<float>(aPower);
	}
	//////////////////////////////////////////////////////////////////////////////
	void C_Material::setDiscard(const bool aDiscard)
	{
		mDiscard = static_cast<bool>(aDiscard);
	}
	//////////////////////////////////////////////////////////////////////////////
	void C_Material::setLighting(const bool aLighting)
	{
		mLighting = static_cast<bool>(aLighting);
	}
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	Vector4 C_Material::getColor() const
	{
		return mColor;
	}
	//////////////////////////////////////////////////////////////////////////////
	Vector3 C_Material::getAmbient() const
	{
		return mAmbient;
	}
	//////////////////////////////////////////////////////////////////////////////
	Vector3 C_Material::getDiffuse() const
	{
		return mDiffuse;
	}
	//////////////////////////////////////////////////////////////////////////////
	Vector3 C_Material::getSpecular() const
	{
		return mSpecular;
	}
	//////////////////////////////////////////////////////////////////////////////
	float C_Material::getShininess() const
	{
		return mShininess;
	}
	//////////////////////////////////////////////////////////////////////////////
	Texture* C_Material::getTexture() const
	{
		return mTexture;
	}
	//////////////////////////////////////////////////////////////////////////////
	Texture* C_Material::getSpecMap() const
	{
		return mSpecMap;
	}
	//////////////////////////////////////////////////////////////////////////////
	Texture* C_Material::getNormMap() const
	{
		return mNormMap;
	}
	//////////////////////////////////////////////////////////////////////////////
	Shader* C_Material::getShader() const
	{
		return mShader;
	}
	//////////////////////////////////////////////////////////////////////////////
	Cubemap* C_Material::getReflection() const
	{
		return mEnvReflection;
	}
	//////////////////////////////////////////////////////////////////////////////
	float C_Material::getReflectionPower() const
	{
		return mReflectionPower;
	}
	//////////////////////////////////////////////////////////////////////////////
	bool C_Material::getDiscard() const
	{
		return mDiscard;
	}
	//////////////////////////////////////////////////////////////////////////////
	bool C_Material::getLighting() const
	{
		return mLighting;
	}
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	int C_Material::getTextureID() const
	{
		return mTextureID;
	}
	//////////////////////////////////////////////////////////////////////////////
	int C_Material::getSpecMapID() const
	{
		return mSpecMapID;
	}
	//////////////////////////////////////////////////////////////////////////////
	int C_Material::getNormMapID() const
	{
		return mNormMapID;
	}
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	bool C_Material::saveToXML(std::string aFile) const
	{
		Serializer::SerializerXML serializer;

		if (!serializer.write(aFile, "Material")) 
		{ Log::error("Can't save Material: " + aFile); return false; }

		if (!serializer.setVector4("Color", mColor, {"R", "G", "B", "A"}))
		{ Log::error("Can't save Material color: " + aFile); return false; }

		if (!serializer.setVector3("Ambient", mAmbient, {"R", "G", "B"}))
		{ Log::error("Can't save Material ambient: " + aFile); return false; }

		if (!serializer.setVector3("Diffuse", mDiffuse, {"R", "G", "B"}))
		{ Log::error("Can't save Material diffuse: " + aFile); return false; }

		if (!serializer.setVector3("Specular", mSpecular, {"R", "G", "B"}))
		{ Log::error("Can't save Material specular: " + aFile); return false; }

		if (!serializer.setFloat("Shininess", mShininess))
		{ Log::error("Can't save Material shininess: " + aFile); return false; }

		if (!serializer.setFloat("ReflectionPower", mReflectionPower))
		{ Log::error("Can't save Material reflection power: " + aFile); return false; }

		if (!serializer.setBool("Discard", mDiscard))
		{ Log::error("Can't save Material discard: " + aFile); return false; }

		if (!serializer.setBool("Lighting", mLighting))
		{ Log::error("Can't save Material lighting: " + aFile); return false; }

		if (!serializer.save())
		{ Log::error("Can't save Material: " + aFile); return false; }

		Log::success("Material saved: " + aFile);

		return true;
	}
	//////////////////////////////////////////////////////////////////////////////
	bool C_Material::saveToJSON(std::string aFile) const
	{
		json j;
		
		j["Material"]["Color"] = {mColor.x, mColor.y, mColor.z, mColor.w};
		j["Material"]["Ambient"] = {mAmbient.x, mAmbient.y, mAmbient.z};
		j["Material"]["Diffuse"] = {mDiffuse.x, mDiffuse.y, mDiffuse.z};
		j["Material"]["Specular"] = {mSpecular.x, mSpecular.y, mSpecular.z};
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
	bool C_Material::loadFromXML(std::string aFile)
	{
		Serializer::SerializerXML serializer;

		if (!serializer.read(aFile, "Material"))
		{ Log::error("Can't load Material: " + aFile); return false; }

		if (!serializer.getVector4("Color", &mColor, {"R", "G", "B", "A"}))
		{ Log::error("Can't load Material color: " + aFile); return false; }

		if (!serializer.getVector3("Ambient", &mAmbient, {"R", "G", "B"}))
		{ Log::error("Can't load Material ambient: " + aFile); return false; }

		if (!serializer.getVector3("Diffuse", &mDiffuse, {"R", "G", "B"}))
		{ Log::error("Can't load Material diffuse: " + aFile); return false; }

		if (!serializer.getVector3("Specular", &mSpecular, {"R", "G", "B"}))
		{ Log::error("Can't load Material specular: " + aFile); return false; }

		if (!serializer.getFloat("Shininess", &mShininess))
		{ Log::error("Can't load Material shininess: " + aFile); return false; }

		if (!serializer.getFloat("ReflectionPower", &mReflectionPower))
		{ Log::error("Can't load Material reflection power: " + aFile); return false; }

		if (!serializer.getBool("Discard", &mDiscard))
		{ Log::error("Can't load Material discard: " + aFile); return false; }

		if (!serializer.getBool("Lighting", &mLighting))
		{ Log::error("Can't load Material lighting: " + aFile); return false; }

		std::string diffuseMapPath = "None";
		std::string specularMapPath = "None";
		std::string normalMapPath = "None";

		serializer.getSubString({"Textures", "Diffuse"}, &diffuseMapPath);
		serializer.getSubString({"Textures", "Specular"}, &specularMapPath);
		serializer.getSubString({"Textures", "Normal"}, &normalMapPath);

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
	bool C_Material::loadFromJSON(std::string aFile)
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

		mAmbient.x = j["Material"]["Ambient"][0];
		mAmbient.y = j["Material"]["Ambient"][1];
		mAmbient.z = j["Material"]["Ambient"][2];

		mDiffuse.x = j["Material"]["Diffuse"][0];
		mDiffuse.y = j["Material"]["Diffuse"][1];
		mDiffuse.z = j["Material"]["Diffuse"][2];

		mSpecular.x = j["Material"]["Specular"][0];
		mSpecular.y = j["Material"]["Specular"][1];
		mSpecular.z = j["Material"]["Specular"][2];

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
	C_Material::~C_Material()
	{

	}

}
