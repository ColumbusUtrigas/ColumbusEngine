/************************************************
*              	   Material.h                   *
*************************************************
*          This file is a part of:              *
*               COLUMBUS ENGINE                 *
*************************************************
*             Nikolay(Columbus) Red             *
*                   20.07.2017                  *
*************************************************/

#pragma once

#include <Math/Vector2.h>
#include <Math/Vector3.h>
#include <Math/Vector4.h>
#include <Graphics/Texture.h>
#include <Graphics/Cubemap.h>
#include <Graphics/Shader.h>
#include <System/System.h>
#include <System/Console.h>

namespace C
{

	class C_Material
	{
	private:
		C_Vector4 color;
		C_Vector3 diffuse;
		C_Vector3 ambient;
		C_Vector3 specular;
		float reflectionPower;
		C_Texture* texture = NULL;
		C_Texture* specmap = NULL;
		C_Texture* normmap = NULL;
		C_Shader* shader = NULL;

		bool discard = false;

		C_Cubemap* envRefl = NULL;

		float shininess = 32;
	public:
		//Constructor
		C_Material();
		//Constructor 2
		C_Material(const char* aFile);
		//Set color
		void setColor(const C_Vector4 aColor);
		//Set light ambient color
		void setAmbient(const C_Vector3 aAmbient);
		//Set light diffuse color
		void setDiffuse(const C_Vector3 aDiffuse);
		//Set light specular color
		void setSpecular(const C_Vector3 aSpecular);
		//Set specular shininess
		void setShininess(const float aShininess);
		//Set diffuse texture
		void setTexture(const C_Texture* aTexture);
		//Set specular texture
		void setSpecMap(const C_Texture* aSpecMap);
		//Set normal texture
		void setNormMap(const C_Texture* aNormMap);
		//Set shader
		void setShader(const C_Shader* aShader);
		//Set cubemap reflection
		void setReflection(const C_Cubemap* aReflection);
		//Set reflection power
		void setReflectionPower(const float aPower);
		//Set discard alpha
		void setDiscard(const bool aDiscard);
		//Return color
		C_Vector4 getColor();
		//Return light ambient color
		C_Vector3 getAmbient();
		//Return light diffuse color
		C_Vector3 getDiffuse();
		//Return light specular color
		C_Vector3 getSpecular();
		//Return specular shininess
		float getShininess();
		//Return diffuse texture
		C_Texture* getTexture();
		//Return specular texture
		C_Texture* getSpecMap();
		//Return normal texture
		C_Texture* getNormMap();
		//Return shader
		C_Shader* getShader();
		//Return cubemap reflection
		C_Cubemap* getReflection();
		//Return reflection power
		float getReflectionPower();
		//Return discard alpha
		bool getDiscard();
		//Serialize to XML file
		void saveToXML(const char* aFile);
		//Deserialize from XML file
		void loadFromXML(const char* aFile);
		//Destructor
		~C_Material();
	};

}
