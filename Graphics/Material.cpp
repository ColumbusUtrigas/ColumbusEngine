/************************************************
*                 Material.cpp                  *
*************************************************
*          This file is a part of:              *
*               COLUMBUS ENGINE                 *
*************************************************
*             Nikolay(Columbus) Red             *
*                   20.07.2017                  *
*************************************************/

#include <Graphics/Material.h>

namespace C
{

	//////////////////////////////////////////////////////////////////////////////
	//Constructor
	C_Material::C_Material()
	{
		color = C_Vector4(1, 1, 1, 1);
		diffuse = C_Vector3(1, 1, 1);
		ambient = C_Vector3(0.25, 0.25, 0.25);
		specular = C_Vector3(1, 1, 1);
		shininess = 32;
		reflectionPower = 0.2;
	}
	//////////////////////////////////////////////////////////////////////////////
	//Constructor 2
	C_Material::C_Material(const char* aFile)
	{
		loadFromXML(aFile);
	}
	//////////////////////////////////////////////////////////////////////////////
	//Set color
	void C_Material::setColor(const C_Vector4 aColor)
	{
		color = (C_Vector4)aColor;
	}
	//////////////////////////////////////////////////////////////////////////////
	//Set light ambient color
	void C_Material::setAmbient(const C_Vector3 aAmbient)
	{
		ambient = (C_Vector3)aAmbient;
	}
	//////////////////////////////////////////////////////////////////////////////
	//Set light diffuse color
	void C_Material::setDiffuse(const C_Vector3 aDiffuse)
	{
		diffuse = (C_Vector3)aDiffuse;
	}
	//////////////////////////////////////////////////////////////////////////////
	//Set light specular color
	void C_Material::setSpecular(const C_Vector3 aSpecular)
	{
		specular = (C_Vector3)aSpecular;
	}
	//////////////////////////////////////////////////////////////////////////////
	//Set specular shininess
	void C_Material::setShininess(const float aShininess)
	{
		shininess = (float)aShininess;
	}
	//////////////////////////////////////////////////////////////////////////////
	//Set diffuse texture
	void C_Material::setTexture(const C_Texture* aTexture)
	{
		texture = (C_Texture*)aTexture;
	}
	//////////////////////////////////////////////////////////////////////////////
	//Set specular texture
	void C_Material::setSpecMap(const C_Texture* aSpecMap)
	{
		specmap = (C_Texture*)aSpecMap;
	}
	//////////////////////////////////////////////////////////////////////////////
	//Set normal texture
	void C_Material::setNormMap(const C_Texture* aNormMap)
	{
		normmap = (C_Texture*)aNormMap;
	}
	//////////////////////////////////////////////////////////////////////////////
	//Set shader
	void C_Material::setShader(const C_Shader* aShader)
	{
		shader = (C_Shader*)aShader;
	}
	//////////////////////////////////////////////////////////////////////////////
	//Set cubemap reflection
	void C_Material::setReflection(const C_Cubemap* aReflecction)
	{
		envRefl = (C_Cubemap*)aReflecction;
	}
	//////////////////////////////////////////////////////////////////////////////
	//Set reflection power
	void C_Material::setReflectionPower(const float aPower)
	{
		reflectionPower = (float)aPower;
	}
	//////////////////////////////////////////////////////////////////////////////
	//Set discard alpha
	void C_Material::setDiscard(const bool aDiscard)
	{
		discard = (bool)aDiscard;
	}
	//////////////////////////////////////////////////////////////////////////////
	//Return color
	C_Vector4 C_Material::getColor()
	{
		return color;
	}
	//////////////////////////////////////////////////////////////////////////////
	//Return light ambient color
	C_Vector3 C_Material::getAmbient()
	{
		return ambient;
	}
	//////////////////////////////////////////////////////////////////////////////
	//Return light diffuer color
	C_Vector3 C_Material::getDiffuse()
	{
		return diffuse;
	}
	//////////////////////////////////////////////////////////////////////////////
	//Return light specular color
	C_Vector3 C_Material::getSpecular()
	{
		return specular;
	}
	//////////////////////////////////////////////////////////////////////////////
	//Return specular shininess
	float C_Material::getShininess()
	{
		return shininess;
	}
	//////////////////////////////////////////////////////////////////////////////
	//Return diffuse texture
	C_Texture* C_Material::getTexture()
	{
		return texture;
	}
	//////////////////////////////////////////////////////////////////////////////
	//Return specular texture
	C_Texture* C_Material::getSpecMap()
	{
		return specmap;
	}
	//////////////////////////////////////////////////////////////////////////////
	C_Texture* C_Material::getNormMap()
	{
		return normmap;
	}
	//////////////////////////////////////////////////////////////////////////////
	//Return shader
	C_Shader* C_Material::getShader()
	{
		return shader;
	}
	//////////////////////////////////////////////////////////////////////////////
	//Return cubemap reflection
	C_Cubemap* C_Material::getReflection()
	{
		return envRefl;
	}
	//////////////////////////////////////////////////////////////////////////////
	//Return reflection power
	float C_Material::getReflectionPower()
	{
		return reflectionPower;
	}
	//////////////////////////////////////////////////////////////////////////////
	//Return discard alpha
	bool C_Material::getDiscard()
	{
		return discard;
	}
	//////////////////////////////////////////////////////////////////////////////
	//Serialize to XML file
	void C_Material::saveToXML(const char* aFile)
	{
		Serializer::C_SerializerXML serializer(aFile, "Material");

		if (!serializer.setVector4("Color", color, {"R", "G", "B", "A"}))
		{ C_Error("Can't save Material color: %s", aFile); return; }

		if (!serializer.setVector3("Ambient", ambient, {"R", "G", "B"}))
		{ C_Error("Can't save Material ambient: %s", aFile); return; }

		if (!serializer.setVector3("Diffuse", diffuse, {"R", "G", "B"}))
		{ C_Error("Can't save Material diffuse: %s", aFile); return; }

		if (!serializer.setVector3("Specular", specular, {"R", "G", "B"}))
		{ C_Error("Can't save Material specular: %s", aFile); return; }

		if (!serializer.setFloat("Shininess", shininess))
		{ C_Error("Can't save Material shininess"); return; }

		if (!serializer.setFloat("ReflectionPower", reflectionPower))
		{ C_Error("Can't save Material reflection power"); return; }

		if (!serializer.setBool("Discard", discard))
		{ C_Error("Can't save Material discard"); return; }

		C_Success("Material saved: %s", aFile);
	}
	//////////////////////////////////////////////////////////////////////////////
	//Deserialize from XML file
	void C_Material::loadFromXML(const char* aFile)
	{
		Serializer::C_SerializerXML serializer(aFile, "Material");

		if (!serializer.load(aFile, "Material")) { C_Error("Can't load Material: %s", aFile); return; }

		if (!serializer.getVector4("Color", &color, {"R", "G", "B", "A"}))
		{ C_Error("Can't load Material color: %s", aFile); return; }

		if (!serializer.getVector3("Ambient", &ambient, {"R", "G", "B"}))
		{ C_Error("Can't load Material ambient: %s", aFile); return; }

		if (!serializer.getVector3("Diffuse", &diffuse, {"R", "G", "B"}))
		{ C_Error("Can't load Material diffuse: %s", aFile); return; }

		if (!serializer.getVector3("Specular", &specular, {"R", "G", "B"}))
		{ C_Error("Can't load Material specular: %s", aFile); return; }

		if (!serializer.getFloat("Shininess", &shininess))
		{ C_Error("Can't load Material shininess: %s", aFile); return; }

		if (!serializer.getFloat("ReflectionPower", &reflectionPower))
		{ C_Error("Can't load Material reflection power: %s", aFile); return; }

		if (!serializer.getBool("Discard", &discard))
		{ C_Error("Can't load Material discard: %s", aFile); return; }

		C_Success("Material loaded: %s", aFile);
	}
	//////////////////////////////////////////////////////////////////////////////
	//Destructor
	C_Material::~C_Material()
	{

	}

}
