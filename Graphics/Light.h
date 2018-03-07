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

		C_Vector3 mPos = C_Vector3(0, 0, 0);
		C_Vector3 mDir = C_Vector3(-0.5, -0.4, -0.3);

		C_Vector3 mColor = C_Vector3(1, 1, 1);

		float mConstant = 1.0;
		float mLinear = 0.09;
		float mQuadratic = 0.032;

		float mInnerCutoff = 12.5;
		float mOuterCutoff = 17.5;
	public:
		C_Light();
		C_Light(const int aType, C_Vector3 aPos = C_Vector3(0, 0, 0));
		C_Light(std::string aFile, C_Vector3 aPos = C_Vector3(0, 0, 0));

		void setType(const int aType);
		void setPos(const C_Vector3 aPos);
		void setDir(const C_Vector3 aDir);
		void setColor(const C_Vector3 aColor);
		void setConstant(const float aConst);
		void setLinear(const float aLinear);
		void setQuadratic(const float aQuadratic);
		void setInnerCutoff(const float aInnerCutoff);
		void setOuterCutoff(const float aOuterCutoff);

		int getType();
		C_Vector3 getPos() const;
		C_Vector3 getDir() const;
		C_Vector3 getColor() const;
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
