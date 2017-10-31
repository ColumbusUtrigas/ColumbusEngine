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
  //Constructor
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
  void C_Light::saveToFile(const char* aFile) const
  {

  }
  //////////////////////////////////////////////////////////////////////////////
  //Deserialize from XML file
  void C_Light::loadFromFile(const char* aFile)
  {

  }
  //////////////////////////////////////////////////////////////////////////////
  //Destructor
  C_Light::~C_Light()
  {

  }

}
