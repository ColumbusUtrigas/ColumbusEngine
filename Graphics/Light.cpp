/************************************************
*                   Light.cpp                   *
*************************************************
*          This file is a part of:              *
*               COLUMBUS ENGINE                 *
*************************************************
*                Nika(Columbus) Red             *
*                   20.07.2017                  *
*************************************************/

#include <Graphics/Light.h>

namespace Columbus
{

	//////////////////////////////////////////////////////////////////////////////
	C_Light::C_Light() :
		mType(0),
		mPos(C_Vector3(0, 0, 0)),
		mDir(C_Vector3(-0.5, -0.4, -0.3)),
		mConstant(1.0),
		mLinear(0.09),
		mQuadratic(0.032),
		mInnerCutoff(12.5),
		mOuterCutoff(17.5)
	{

	}
	//////////////////////////////////////////////////////////////////////////////
	C_Light::C_Light(const int aType, C_Vector3 aPos) :
		mType(aType),
		mPos(aPos),
		mDir(C_Vector3(-0.5, -0.4, -0.3)),
		mConstant(1.0),
		mLinear(0.09),
		mQuadratic(0.032),
		mInnerCutoff(12.5),
		mOuterCutoff(17.5)
	{
    
	}
	//////////////////////////////////////////////////////////////////////////////
	C_Light::C_Light(std::string aFile, C_Vector3 aPos) :
		mType(0),
		mPos(aPos),
		mDir(C_Vector3(-0.5, -0.4, -0.3)),
		mConstant(1.0),
		mLinear(0.09),
		mQuadratic(0.032),
		mInnerCutoff(12.5),
		mOuterCutoff(17.5)
	{
		load(aFile);
	}
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	void C_Light::setType(const int aType)
	{
		mType = static_cast<int>(aType);
	}
	//////////////////////////////////////////////////////////////////////////////
	void C_Light::setPos(const C_Vector3 aPos)
	{
		mPos = static_cast<C_Vector3>(aPos);
	}
	//////////////////////////////////////////////////////////////////////////////
	void C_Light::setDir(const C_Vector3 aDir)
	{
		mDir = static_cast<C_Vector3>(aDir);
	}
	//////////////////////////////////////////////////////////////////////////////
	void C_Light::setColor(const C_Vector3 aColor)
	{
		mColor = static_cast<C_Vector3>(aColor);
	}
	//////////////////////////////////////////////////////////////////////////////
	void C_Light::setConstant(const float aConstant)
	{
		mConstant = static_cast<float>(aConstant);
	}
	//////////////////////////////////////////////////////////////////////////////
	void C_Light::setLinear(const float aLinear)
	{
		mLinear = static_cast<float>(aLinear);
	}
	//////////////////////////////////////////////////////////////////////////////
	void C_Light::setQuadratic(const float aQuadratic)
	{
		mQuadratic = static_cast<float>(aQuadratic);
	}
	//////////////////////////////////////////////////////////////////////////////
	void C_Light::setInnerCutoff(const float aInnerCutoff)
	{
		mInnerCutoff = static_cast<float>(aInnerCutoff);
	}
	//////////////////////////////////////////////////////////////////////////////
	void C_Light::setOuterCutoff(const float aOuterCutoff)
	{
		mOuterCutoff = static_cast<float>(aOuterCutoff);
	}
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	int C_Light::getType()
	{
		return mType;
	}
	//////////////////////////////////////////////////////////////////////////////
	C_Vector3 C_Light::getPos() const
	{
		return mPos;
	}
	//////////////////////////////////////////////////////////////////////////////
	C_Vector3 C_Light::getDir() const
	{
		return mDir;
	}
	//////////////////////////////////////////////////////////////////////////////
	C_Vector3 C_Light::getColor() const
	{
		return mColor;
	}
	//////////////////////////////////////////////////////////////////////////////
	float C_Light::getConstant() const
	{
		return mConstant;
	}
	//////////////////////////////////////////////////////////////////////////////
	float C_Light::getLinear() const
	{
		return mLinear;
	}
	//////////////////////////////////////////////////////////////////////////////
	float C_Light::getQuadratic() const
	{
		return mQuadratic;
	}
	//////////////////////////////////////////////////////////////////////////////
	float C_Light::getInnerCutoff() const
	{
		return mInnerCutoff;
	}
	//////////////////////////////////////////////////////////////////////////////
	float C_Light::getOuterCutoff() const
	{
		return mOuterCutoff;
	}
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	bool C_Light::saveToXML(std::string aFile) const
	{
		Serializer::C_SerializerXML serializer;

		if (!serializer.write(aFile, "Light"))
		{ C_Log::error("Can't save Light: " + aFile); return false; }

		if (!serializer.setInt("Type", mType))
		{ C_Log::error("Can't save Light type: " + aFile); return false; }

		if (!serializer.setVector3("Direction", mDir, { "X", "Y", "Z" }))
		{ C_Log::error("Can't save Light direction: " + aFile); return false; }

		if (!serializer.setVector3("Color", mColor, { "R", "G", "B" }))
		{ C_Log::error("Can't save Light color: " + aFile); return false; }

		if (!serializer.setFloat("Constant", mConstant))
		{ C_Log::error("Can't save Light constant: " + aFile); return false; }

		if (!serializer.setFloat("Linear", mLinear))
		{ C_Log::error("Can't save Light linear: " + aFile); return false; }

		if (!serializer.setFloat("Quadratic", mQuadratic))
		{ C_Log::error("Can't save Light quadratic: " + aFile); return false; }

		if (!serializer.setFloat("InnerCutoff", mInnerCutoff))
		{ C_Log::error("Can't save Light inner cutoff: " + aFile); return false; }

		if (!serializer.setFloat("OuterCutoff", mOuterCutoff))
		{ C_Log::error("Can't save Light outer cutoff: " + aFile); return false; }

		if (!serializer.save())
		{ C_Log::error("Can't save Light: " + aFile); return false; }

		C_Log::success("Light saved: " + aFile);

		return true;
	}
	//////////////////////////////////////////////////////////////////////////////
	bool C_Light::loadFromXML(std::string aFile)
	{
		Serializer::C_SerializerXML serializer;

		if (!serializer.read(aFile, "Light"))
		{ C_Log::error("Can't load Light: " + aFile); return false; }

		if (!serializer.getInt("Type", &mType))
		{ C_Log::error("Can't load Light type: %s" + aFile); return false; }

		if (!serializer.getVector3("Direction", &mDir, { "X", "Y", "Z" }))
		{ C_Log::error("Can't load Light direction: %s" + aFile); return false; }

		if (!serializer.getVector3("Color", &mColor, { "R", "G", "B" }))
		{ C_Log::error("Can't load Light color: " + aFile); return false; }

		if (!serializer.getFloat("Constant", &mConstant))
		{ C_Log::error("Can't load Light constant: " + aFile); return false; }

		if (!serializer.getFloat("Linear", &mLinear))
		{ C_Log::error("Can't load Light linear: " + aFile); return false; }

		if (!serializer.getFloat("Quadratic", &mQuadratic))
		{ C_Log::error("Can't load Light quadratic: " + aFile); return false; }

		if (!serializer.getFloat("InnerCutoff", &mInnerCutoff))
		{ C_Log::error("Can't load Light inner cutoff: " + aFile); return false; }

		if (!serializer.getFloat("OuterCutoff", &mInnerCutoff))
		{ C_Log::error("Can't load Light outer cutoff: " + aFile); return false; }

		C_Log::success("Light loaded: " + aFile);

		return true;
	}
	//////////////////////////////////////////////////////////////////////////////
	bool C_Light::load(std::string aFile)
	{
		if (aFile.find_last_of(".cxlig") != std::string::npos)
			return loadFromXML(aFile);
		else return false;
	}
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	C_Light::~C_Light()
	{

	}

}
