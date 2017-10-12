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
  C_Light::C_Light(const int aType)
  {
    mType = (int)aType;
  }
  //////////////////////////////////////////////////////////////////////////////
  //Set light type
  void C_Light::setType(const int aType)
  {
    mType = (int)aType;
  }
  //////////////////////////////////////////////////////////////////////////////
  //Set light position
  void C_Light::setPos(const C_Vector3 aPos)
  {
    mPos = (C_Vector3)aPos;
  }
  //////////////////////////////////////////////////////////////////////////////
  //Set light direction
  void C_Light::setDir(const C_Vector3 aDir)
  {
    mDir = (C_Vector3)aDir;
  }
  //////////////////////////////////////////////////////////////////////////////
  //Set light color
  void C_Light::setColor(const C_Vector3 aColor)
  {
    mColor = (C_Vector3)aColor;
  }
  //////////////////////////////////////////////////////////////////////////////
  //Set constant attenuation
  void C_Light::setConstant(const float aConstant)
  {
    mConstant = (float)aConstant;
  }
  //////////////////////////////////////////////////////////////////////////////
  //Set linear attenuation
  void C_Light::setLinear(const float aLinear)
  {
    mLinear = (float)aLinear;
  }
  //////////////////////////////////////////////////////////////////////////////
  //Set quadratic attenuation
  void C_Light::setQuadratic(const float aQuadratic)
  {
    mQuadratic = (float)aQuadratic;
  }
  //////////////////////////////////////////////////////////////////////////////
  //Set spotlight inner cutoff
  void C_Light::setInnerCutoff(const float aInnerCutoff)
  {
    mInnerCutoff = (float)aInnerCutoff;
  }
  //////////////////////////////////////////////////////////////////////////////
  //Set spotlight outer cutoff
  void C_Light::setOuterCutoff(const float aOuterCutoff)
  {
    mOuterCutoff = (float)aOuterCutoff;
  }
  //////////////////////////////////////////////////////////////////////////////
  //Return light type
  int C_Light::getType()
  {
    return mType;
  }
  //////////////////////////////////////////////////////////////////////////////
  //Return light position
  C_Vector3 C_Light::getPos()
  {
    return mPos;
  }
  //////////////////////////////////////////////////////////////////////////////
  //Return light direction
  C_Vector3 C_Light::getDir()
  {
    return mDir;
  }
  //////////////////////////////////////////////////////////////////////////////
  //Return light color
  C_Vector3 C_Light::getColor()
  {
    return mColor;
  }
  //////////////////////////////////////////////////////////////////////////////
  //Return constant attenuation
  float C_Light::getConstant()
  {
    return mConstant;
  }
  //////////////////////////////////////////////////////////////////////////////
  //Return linear attenuation
  float C_Light::getLinear()
  {
    return mLinear;
  }
  //////////////////////////////////////////////////////////////////////////////
  //Return quadratic attenuation
  float C_Light::getQuadratic()
  {
    return mQuadratic;
  }
  //////////////////////////////////////////////////////////////////////////////
  //Return spotlight inner cutoff
  float C_Light::getInnerCutoff()
  {
    return mInnerCutoff;
  }
  //////////////////////////////////////////////////////////////////////////////
  //Return spotlight outer cutoff
  float C_Light::getOuterCutoff()
  {
    return mOuterCutoff;
  }
  //////////////////////////////////////////////////////////////////////////////
  //Serialize to XML file
  void C_Light::saveToFile(const char* aFile)
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
