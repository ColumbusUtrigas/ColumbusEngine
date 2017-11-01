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
#include <System/Serializer.h>

namespace C
{

	class C_Material
	{
	private:
		C_Vector4 mColor = C_Vector4(1, 1, 1, 1);
		C_Vector3 mDiffuse = C_Vector3(1, 1, 1);
		C_Vector3 mAmbient = C_Vector3(0.25, 0.25, 0.25);
		C_Vector3 mSpecular = C_Vector3(1, 1, 1);
		float mReflectionPower = 0.2;
		C_Texture* mTexture = nullptr;
		C_Texture* mSpecMap = nullptr;
		C_Texture* mNormMap = nullptr;
		C_Shader* mShader = nullptr;

		bool mDiscard = false;

		C_Cubemap* mEnvReflection = nullptr;

		float mShininess = 32;
	public:
		//Constructor
		C_Material();
		//Constructor 2
		C_Material(std::string aFile);
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
		C_Vector4 getColor() const;
		//Return light ambient color
		C_Vector3 getAmbient() const;
		//Return light diffuse color
		C_Vector3 getDiffuse() const;
		//Return light specular color
		C_Vector3 getSpecular() const;
		//Return specular shininess
		float getShininess() const;
		//Return diffuse texture
		C_Texture* getTexture() const;
		//Return specular texture
		C_Texture* getSpecMap() const;
		//Return normal texture
		C_Texture* getNormMap() const;
		//Return shader
		C_Shader* getShader() const;
		//Return cubemap reflection
		C_Cubemap* getReflection() const;
		//Return reflection power
		float getReflectionPower() const;
		//Return discard alpha
		bool getDiscard() const;

		//Serialize to XML file
		bool saveToXML(std::string aFile) const;
		//Deserialize from XML file
		bool loadFromXML(std::string aFile);
		//Destructor
		~C_Material();
	};

}
