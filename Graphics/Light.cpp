/************************************************
*                   Light.cpp                   *
*************************************************
*          This file is a part of:              *
*               COLUMBUS ENGINE                 *
*************************************************
*             Nikolay(Columbus) Red             *
*                   20.07.2017                  *
*************************************************/

#include <Graphics/Light.h>

namespace C
{

	//////////////////////////////////////////////////////////////////////////////
	//Constructor 1
	C_Light::C_Light() :
		mType(0),
		mConstant(1.0),
		mLinear(0.09),
		mQuadratic(0.032),
		mInnerCutoff(12.5),
		mOuterCutoff(17.5)
	{

	}
	//////////////////////////////////////////////////////////////////////////////
	//Constructor 2
	C_Light::C_Light(const int aType) :
		mType(aType),
		mConstant(1.0),
		mLinear(0.09),
		mQuadratic(0.032),
		mInnerCutoff(12.5),
		mOuterCutoff(17.5)
	{
    
	}
	//////////////////////////////////////////////////////////////////////////////
	//Constructor 3
	C_Light::C_Light(std::string aFile) :
		mType(0),
		mConstant(1.0),
		mLinear(0.09),
		mQuadratic(0.032),
		mInnerCutoff(12.5),
		mOuterCutoff(17.5)
	{
		loadFromFile(aFile);
	}
	//////////////////////////////////////////////////////////////////////////////
	//Set light type
	void C_Light::setType(const int aType)
	{
		mType = static_cast<int>(aType);
	}
	//////////////////////////////////////////////////////////////////////////////
	//Set light position
	void C_Light::setPos(const C_Vector3 aPos)
	{
		mPos = static_cast<C_Vector3>(aPos);
	}
	//////////////////////////////////////////////////////////////////////////////
	//Set light direction
	void C_Light::setDir(const C_Vector3 aDir)
	{
		mDir = static_cast<C_Vector3>(aDir);
	}
	//////////////////////////////////////////////////////////////////////////////
	//Set light color
	void C_Light::setColor(const C_Vector3 aColor)
	{
		mColor = static_cast<C_Vector3>(aColor);
	}
	//////////////////////////////////////////////////////////////////////////////
	//Set constant attenuation
	void C_Light::setConstant(const float aConstant)
	{
		mConstant = static_cast<float>(aConstant);
	}
	//////////////////////////////////////////////////////////////////////////////
	//Set linear attenuation
	void C_Light::setLinear(const float aLinear)
	{
		mLinear = static_cast<float>(aLinear);
	}
	//////////////////////////////////////////////////////////////////////////////
	//Set quadratic attenuation
	void C_Light::setQuadratic(const float aQuadratic)
	{
		mQuadratic = static_cast<float>(aQuadratic);
	}
	//////////////////////////////////////////////////////////////////////////////
	//Set spotlight inner cutoff
	void C_Light::setInnerCutoff(const float aInnerCutoff)
	{
		mInnerCutoff = static_cast<float>(aInnerCutoff);
	}
	//////////////////////////////////////////////////////////////////////////////
	//Set spotlight outer cutoff
	void C_Light::setOuterCutoff(const float aOuterCutoff)
	{
		mOuterCutoff = static_cast<float>(aOuterCutoff);
	}
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//Return light type
	int C_Light::getType()
	{
		return mType;
	}
	//////////////////////////////////////////////////////////////////////////////
	//Return light position
	C_Vector3 C_Light::getPos() const
	{
		return mPos;
	}
	//////////////////////////////////////////////////////////////////////////////
	//Return light direction
	C_Vector3 C_Light::getDir() const
	{
		return mDir;
	}
	//////////////////////////////////////////////////////////////////////////////
	//Return light color
	C_Vector3 C_Light::getColor() const
	{
		return mColor;
	}
	//////////////////////////////////////////////////////////////////////////////
	//Return constant attenuation
	float C_Light::getConstant() const
	{
		return mConstant;
	}
	//////////////////////////////////////////////////////////////////////////////
	//Return linear attenuation
	float C_Light::getLinear() const
	{
		return mLinear;
	}
	//////////////////////////////////////////////////////////////////////////////
	//Return quadratic attenuation
	float C_Light::getQuadratic() const
	{
		return mQuadratic;
	}
	//////////////////////////////////////////////////////////////////////////////
	//Return spotlight inner cutoff
	float C_Light::getInnerCutoff() const
	{
		return mInnerCutoff;
	}
	//////////////////////////////////////////////////////////////////////////////
	//Return spotlight outer cutoff
	float C_Light::getOuterCutoff() const
	{
		return mOuterCutoff;
	}
	//////////////////////////////////////////////////////////////////////////////
	//Serialize to XML file
	bool C_Light::saveToFile(std::string aFile) const
	{
		Serializer::C_SerializerXML serializer;

		if (!serializer.write(aFile, "Light"))
		{ C_Error("Can't save Light: %s", aFile); return false; }

		if (!serializer.setInt("Type", mType))
		{ C_Error("Can't save Light type: %s", aFile); return false; }

		if (!serializer.setVector3("Position", mPos, {"X", "Y", "Z"}))
		{ C_Error("Can't save Light position: %s", aFile); return false; }

		if (!serializer.setVector3("Direction", mDir, { "X", "Y", "Z" }))
		{ C_Error("Can't save Light direction: %s", aFile); return false; }

		if (!serializer.setVector3("Color", mColor, { "R", "G", "B" }))
		{ C_Error("Can't save Light color: %s", aFile); return false; }

		if (!serializer.setFloat("Constant", mConstant))
		{ C_Error("Can't save Light constant: %s", aFile); return false; }

		if (!serializer.setFloat("Linear", mLinear))
		{ C_Error("Can't save Light linear: %s", aFile); return false; }

		if (!serializer.setFloat("Quadratic", mQuadratic))
		{ C_Error("Can't save Light quadratic: %s", aFile); return false; }

		if (!serializer.setFloat("InnerCutoff", mInnerCutoff))
		{ C_Error("Can't save Light inner cutoff: %s", aFile); return false; }

		if (!serializer.setFloat("OuterCutoff", mOuterCutoff))
		{ C_Error("Can't save Light outer cutoff: %s", aFile); return false; }

		if (!serializer.save())
		{ C_Error("Can't save Light: %s", aFile); return false; }

		C_Success("Light saved: %s", aFile);

		return true;
	}
	//////////////////////////////////////////////////////////////////////////////
	//Deserialize from XML file
	bool C_Light::loadFromFile(std::string aFile)
	{
		Serializer::C_SerializerXML serializer;

		if (!serializer.read(aFile, "Light"))
		{ C_Error("Can't load Light: %s", aFile); return false; }

		if (!serializer.getInt("Type", &mType))
		{ C_Error("Can't load Light type: %s", aFile); return false; }

		if (!serializer.getVector3("Position", &mPos, { "X", "Y", "Z" }))
		{ C_Error("Can't load Light position: %s", aFile); return false; }

		if (!serializer.getVector3("Direction", &mDir, { "X", "Y", "Z" }))
		{ C_Error("Can't load Light direction: %s", aFile); return false; }

		if (!serializer.getVector3("Color", &mColor, { "R", "G", "B" }))
		{ C_Error("Can't load Light color: %s", aFile); return false; }

		if (!serializer.getFloat("Constant", &mConstant))
		{ C_Error("Can't load Light constant: %s", aFile); return false; }

		if (!serializer.getFloat("Linear", &mLinear))
		{ C_Error("Can't load Light linear: %s", aFile); return false; }

		if (!serializer.getFloat("Quadratic", &mQuadratic))
		{ C_Error("Can't load Light quadratic: %s", aFile); return false; }

		if (!serializer.getFloat("InnerCutoff", &mInnerCutoff))
		{ C_Error("Can't load Light inner cutoff: %s", aFile); return false; }

		if (!serializer.getFloat("OuterCutoff", &mInnerCutoff))
		{ C_Error("Can't load Light outer cutoff: %s", aFile); return false; }

		C_Success("Light loaded %s", aFile);

		return true;
	}
	//////////////////////////////////////////////////////////////////////////////
	//Destructor
	C_Light::~C_Light()
	{

	}

}
