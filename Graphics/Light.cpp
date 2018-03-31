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
	Light::Light() :
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
	Light::Light(const int aType, Vector3 aPos) :
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
	Light::Light(std::string aFile, Vector3 aPos) :
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
	void Light::setType(const int aType)
	{
		mType = static_cast<int>(aType);
	}
	//////////////////////////////////////////////////////////////////////////////
	void Light::setPos(const Vector3 aPos)
	{
		mPos = static_cast<Vector3>(aPos);
	}
	//////////////////////////////////////////////////////////////////////////////
	void Light::setDir(const Vector3 aDir)
	{
		mDir = static_cast<Vector3>(aDir);
	}
	//////////////////////////////////////////////////////////////////////////////
	void Light::setColor(const Vector3 aColor)
	{
		mColor = static_cast<Vector3>(aColor);
	}
	//////////////////////////////////////////////////////////////////////////////
	void Light::setConstant(const float aConstant)
	{
		mConstant = static_cast<float>(aConstant);
	}
	//////////////////////////////////////////////////////////////////////////////
	void Light::setLinear(const float aLinear)
	{
		mLinear = static_cast<float>(aLinear);
	}
	//////////////////////////////////////////////////////////////////////////////
	void Light::setQuadratic(const float aQuadratic)
	{
		mQuadratic = static_cast<float>(aQuadratic);
	}
	//////////////////////////////////////////////////////////////////////////////
	void Light::setInnerCutoff(const float aInnerCutoff)
	{
		mInnerCutoff = static_cast<float>(aInnerCutoff);
	}
	//////////////////////////////////////////////////////////////////////////////
	void Light::setOuterCutoff(const float aOuterCutoff)
	{
		mOuterCutoff = static_cast<float>(aOuterCutoff);
	}
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	int Light::getType()
	{
		return mType;
	}
	//////////////////////////////////////////////////////////////////////////////
	Vector3 Light::getPos() const
	{
		return mPos;
	}
	//////////////////////////////////////////////////////////////////////////////
	Vector3 Light::getDir() const
	{
		return mDir;
	}
	//////////////////////////////////////////////////////////////////////////////
	Vector3 Light::getColor() const
	{
		return mColor;
	}
	//////////////////////////////////////////////////////////////////////////////
	float Light::getConstant() const
	{
		return mConstant;
	}
	//////////////////////////////////////////////////////////////////////////////
	float Light::getLinear() const
	{
		return mLinear;
	}
	//////////////////////////////////////////////////////////////////////////////
	float Light::getQuadratic() const
	{
		return mQuadratic;
	}
	//////////////////////////////////////////////////////////////////////////////
	float Light::getInnerCutoff() const
	{
		return mInnerCutoff;
	}
	//////////////////////////////////////////////////////////////////////////////
	float Light::getOuterCutoff() const
	{
		return mOuterCutoff;
	}
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	bool Light::saveToXML(std::string aFile) const
	{
		Serializer::SerializerXML Serializer;

		if (!Serializer.Write(aFile, "Light"))
		{ Log::error("Can't save Light: " + aFile); return false; }

		if (!Serializer.SetInt("Type", mType))
		{ Log::error("Can't save Light type: " + aFile); return false; }

		if (!Serializer.SetVector3("Direction", mDir, { "X", "Y", "Z" }))
		{ Log::error("Can't save Light direction: " + aFile); return false; }

		if (!Serializer.SetVector3("Color", mColor, { "R", "G", "B" }))
		{ Log::error("Can't save Light color: " + aFile); return false; }

		if (!Serializer.SetFloat("Constant", mConstant))
		{ Log::error("Can't save Light constant: " + aFile); return false; }

		if (!Serializer.SetFloat("Linear", mLinear))
		{ Log::error("Can't save Light linear: " + aFile); return false; }

		if (!Serializer.SetFloat("Quadratic", mQuadratic))
		{ Log::error("Can't save Light quadratic: " + aFile); return false; }

		if (!Serializer.SetFloat("InnerCutoff", mInnerCutoff))
		{ Log::error("Can't save Light inner cutoff: " + aFile); return false; }

		if (!Serializer.SetFloat("OuterCutoff", mOuterCutoff))
		{ Log::error("Can't save Light outer cutoff: " + aFile); return false; }

		if (!Serializer.Save())
		{ Log::error("Can't save Light: " + aFile); return false; }

		Log::success("Light saved: " + aFile);

		return true;
	}
	//////////////////////////////////////////////////////////////////////////////
	bool Light::loadFromXML(std::string aFile)
	{
		Serializer::SerializerXML Serializer;

		if (!Serializer.Read(aFile, "Light"))
		{ Log::error("Can't load Light: " + aFile); return false; }

		if (!Serializer.GetInt("Type", &mType))
		{ Log::error("Can't load Light type: %s" + aFile); return false; }

		if (!Serializer.GetVector3("Direction", &mDir, { "X", "Y", "Z" }))
		{ Log::error("Can't load Light direction: %s" + aFile); return false; }

		if (!Serializer.GetVector3("Color", &mColor, { "R", "G", "B" }))
		{ Log::error("Can't load Light color: " + aFile); return false; }

		if (!Serializer.GetFloat("Constant", &mConstant))
		{ Log::error("Can't load Light constant: " + aFile); return false; }

		if (!Serializer.GetFloat("Linear", &mLinear))
		{ Log::error("Can't load Light linear: " + aFile); return false; }

		if (!Serializer.GetFloat("Quadratic", &mQuadratic))
		{ Log::error("Can't load Light quadratic: " + aFile); return false; }

		if (!Serializer.GetFloat("InnerCutoff", &mInnerCutoff))
		{ Log::error("Can't load Light inner cutoff: " + aFile); return false; }

		if (!Serializer.GetFloat("OuterCutoff", &mInnerCutoff))
		{ Log::error("Can't load Light outer cutoff: " + aFile); return false; }

		Log::success("Light loaded: " + aFile);

		return true;
	}
	//////////////////////////////////////////////////////////////////////////////
	bool Light::load(std::string aFile)
	{
		if (aFile.find_last_of(".cxlig") != std::string::npos)
			return loadFromXML(aFile);
		else return false;
	}
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	Light::~Light()
	{

	}

}
