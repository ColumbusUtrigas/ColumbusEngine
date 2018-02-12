/************************************************
*              	   Material.h                   *
*************************************************
*          This file is a part of:              *
*               COLUMBUS ENGINE                 *
*************************************************
*                Nika(Columbus) Red             *
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
#include <System/Log.h>
#include <System/Serializer.h>
#include <fstream>
#include <json.hpp>

namespace Columbus
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
		bool mLighting = true;

		C_Cubemap* mEnvReflection = nullptr;

		float mShininess = 32;

		int mTextureID = -1;
		int mSpecMapID = -1;
		int mNormMapID = -1;
	public:
		C_Material();
		C_Material(std::string aFile);

		void setColor(const C_Vector4 aColor);
		void setAmbient(const C_Vector3 aAmbient);
		void setDiffuse(const C_Vector3 aDiffuse);
		void setSpecular(const C_Vector3 aSpecular);
		void setShininess(const float aShininess);
		void setTexture(const C_Texture* aTexture);
		void setSpecMap(const C_Texture* aSpecMap);
		void setNormMap(const C_Texture* aNormMap);
		void setShader(const C_Shader* aShader);
		void setReflection(const C_Cubemap* aReflection);
		void setReflectionPower(const float aPower);
		void setDiscard(const bool aDiscard);
		void setLighting(const bool aLighting);

		C_Vector4 getColor() const;
		C_Vector3 getAmbient() const;
		C_Vector3 getDiffuse() const;
		C_Vector3 getSpecular() const;
		float getShininess() const;
		C_Texture* getTexture() const;
		C_Texture* getSpecMap() const;
		C_Texture* getNormMap() const;
		C_Shader* getShader() const;
		C_Cubemap* getReflection() const;
		float getReflectionPower() const;
		bool getDiscard() const;
		bool getLighting() const;

		int getTextureID() const;
		int getSpecMapID() const;
		int getNormMapID() const;

		bool saveToXML(std::string aFile) const;
		bool saveToJSON(std::string aFile) const;
		bool loadFromXML(std::string aFile);
		bool loadFromJSON(std::string aFile);

		~C_Material();
	};

}
