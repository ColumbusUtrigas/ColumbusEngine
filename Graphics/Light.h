/************************************************
*                   Light.h                     *
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
#include <System/System.h>
#include <System/Log.h>
#include <System/Serializer.h>

namespace Columbus
{

	class C_Light
	{
	private:
		int mType = 0;

		Vector3 mPos = Vector3(0, 0, 0);
		Vector3 mDir = Vector3(-0.5, -0.4, -0.3);

		Vector3 mColor = Vector3(1, 1, 1);

		float mConstant = 1.0;
		float mLinear = 0.09;
		float mQuadratic = 0.032;

		float mInnerCutoff = 12.5;
		float mOuterCutoff = 17.5;
	public:
		C_Light();
		C_Light(const int aType, Vector3 aPos = Vector3(0, 0, 0));
		C_Light(std::string aFile, Vector3 aPos = Vector3(0, 0, 0));

		void setType(const int aType);
		void setPos(const Vector3 aPos);
		void setDir(const Vector3 aDir);
		void setColor(const Vector3 aColor);
		void setConstant(const float aConst);
		void setLinear(const float aLinear);
		void setQuadratic(const float aQuadratic);
		void setInnerCutoff(const float aInnerCutoff);
		void setOuterCutoff(const float aOuterCutoff);

		int getType();
		Vector3 getPos() const;
		Vector3 getDir() const;
		Vector3 getColor() const;
		float getConstant() const;
		float getLinear() const;
		float getQuadratic() const;
		float getInnerCutoff() const;
		float getOuterCutoff() const;

		bool saveToXML(std::string aFile) const;
		bool loadFromXML(std::string aFile);
		bool load(std::string aFile);
		
		~C_Light();
	};

}
