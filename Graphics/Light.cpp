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
		mPos(Vector3(0, 0, 0)),
		mDir(Vector3(-0.5, -0.4, -0.3)),
		mConstant(1.0),
		mLinear(0.09),
		mQuadratic(0.032),
		mInnerCutoff(12.5),
		mOuterCutoff(17.5)
	{

	}
	//////////////////////////////////////////////////////////////////////////////
	C_Light::C_Light(const int aType, Vector3 aPos) :
		mType(aType),
		mPos(aPos),
		mDir(Vector3(-0.5, -0.4, -0.3)),
		mConstant(1.0),
		mLinear(0.09),
		mQuadratic(0.032),
		mInnerCutoff(12.5),
		mOuterCutoff(17.5)
	{
    
	}
	//////////////////////////////////////////////////////////////////////////////
	C_Light::C_Light(std::string aFile, Vector3 aPos) :
		mType(0),
		mPos(aPos),
		mDir(Vector3(-0.5, -0.4, -0.3)),
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
	void C_Light::setPos(const Vector3 aPos)
	{
		mPos = static_cast<Vector3>(aPos);
	}
	//////////////////////////////////////////////////////////////////////////////
	void C_Light::setDir(const Vector3 aDir)
	{
		mDir = static_cast<Vector3>(aDir);
	}
	//////////////////////////////////////////////////////////////////////////////
	void C_Light::setColor(const Vector3 aColor)
	{
		mColor = static_cast<Vector3>(aColor);
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
	Vector3 C_Light::getPos() const
	{
		return mPos;
	}
	//////////////////////////////////////////////////////////////////////////////
	Vector3 C_Light::getDir() const
	{
		return mDir;
	}
	//////////////////////////////////////////////////////////////////////////////
	Vector3 C_Light::getColor() const
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
		Serializer::SerializerXML serializer;

		if (!serializer.write(aFile, "Light"))
		{ Log::error("Can't save Light: " + aFile); return false; }

		if (!serializer.setInt("Type", mType))
		{ Log::error("Can't save Light type: " + aFile); return false; }

		if (!serializer.setVector3("Direction", mDir, { "X", "Y", "Z" }))
		{ Log::error("Can't save Light direction: " + aFile); return false; }

		if (!serializer.setVector3("Color", mColor, { "R", "G", "B" }))
		{ Log::error("Can't save Light color: " + aFile); return false; }

		if (!serializer.setFloat("Constant", mConstant))
		{ Log::error("Can't save Light constant: " + aFile); return false; }

		if (!serializer.setFloat("Linear", mLinear))
		{ Log::error("Can't save Light linear: " + aFile); return false; }

		if (!serializer.setFloat("Quadratic", mQuadratic))
		{ Log::error("Can't save Light quadratic: " + aFile); return false; }

		if (!serializer.setFloat("InnerCutoff", mInnerCutoff))
		{ Log::error("Can't save Light inner cutoff: " + aFile); return false; }

		if (!serializer.setFloat("OuterCutoff", mOuterCutoff))
		{ Log::error("Can't save Light outer cutoff: " + aFile); return false; }

		if (!serializer.save())
		{ Log::error("Can't save Light: " + aFile); return false; }

		Log::success("Light saved: " + aFile);

		return true;
	}
	//////////////////////////////////////////////////////////////////////////////
	bool C_Light::loadFromXML(std::string aFile)
	{
		Serializer::SerializerXML serializer;

		if (!serializer.read(aFile, "Light"))
		{ Log::error("Can't load Light: " + aFile); return false; }

		if (!serializer.getInt("Type", &mType))
		{ Log::error("Can't load Light type: %s" + aFile); return false; }

		if (!serializer.getVector3("Direction", &mDir, { "X", "Y", "Z" }))
		{ Log::error("Can't load Light direction: %s" + aFile); return false; }

		if (!serializer.getVector3("Color", &mColor, { "R", "G", "B" }))
		{ Log::error("Can't load Light color: " + aFile); return false; }

		if (!serializer.getFloat("Constant", &mConstant))
		{ Log::error("Can't load Light constant: " + aFile); return false; }

		if (!serializer.getFloat("Linear", &mLinear))
		{ Log::error("Can't load Light linear: " + aFile); return false; }

		if (!serializer.getFloat("Quadratic", &mQuadratic))
		{ Log::error("Can't load Light quadratic: " + aFile); return false; }

		if (!serializer.getFloat("InnerCutoff", &mInnerCutoff))
		{ Log::error("Can't load Light inner cutoff: " + aFile); return false; }

		if (!serializer.getFloat("OuterCutoff", &mInnerCutoff))
		{ Log::error("Can't load Light outer cutoff: " + aFile); return false; }

		Log::success("Light loaded: " + aFile);

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
