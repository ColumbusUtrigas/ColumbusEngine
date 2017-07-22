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
    type = (int)aType;
    pos = C_Vector3(0, 0, 5);
    dir = C_Vector3(-0.5, -0.4f, -0.3f);
    ambient = C_Vector3(0.3f, 0.3f, 0.3f);
    diffuse =  C_Vector3(0.7f, 0.7f, 0.7f);
    specular = C_Vector3(1.0f, 1.0f, 1.0f);
    constant = 1.0;
    linear = 0.09;
    quadratic = 0.032;
    cutoff = 12.5;
  }
  //////////////////////////////////////////////////////////////////////////////
  //Set light type
  void C_Light::setType(const int aType)
  {
    type = (int)aType;
  }
  //////////////////////////////////////////////////////////////////////////////
  //Set light position
  void C_Light::setPos(const C_Vector3 aPos)
  {
    pos = (C_Vector3)aPos;
  }
  //////////////////////////////////////////////////////////////////////////////
  //Set light direction
  void C_Light::setDir(const C_Vector3 aDir)
  {
    dir = (C_Vector3)aDir;
  }
  //////////////////////////////////////////////////////////////////////////////
  //Set ambient color
  void C_Light::setAmbient(const C_Vector3 aAmbient)
  {
    ambient = (C_Vector3)aAmbient;
  }
  //////////////////////////////////////////////////////////////////////////////
  //Set diffuse color
  void C_Light::setDiffuse(const C_Vector3 aDiffuse)
  {
    diffuse = (C_Vector3)aDiffuse;
  }
  //////////////////////////////////////////////////////////////////////////////
  //Set specular color
  void C_Light::setSpecular(const C_Vector3 aSpecular)
  {
    specular = (C_Vector3)aSpecular;
  }
  //////////////////////////////////////////////////////////////////////////////
  //Set constant attenuation
  void C_Light::setConstant(const float aConstant)
  {
    constant = (float)aConstant;
  }
  //////////////////////////////////////////////////////////////////////////////
  //Set linear attenuation
  void C_Light::setLinear(const float aLinear)
  {
    linear = (float)aLinear;
  }
  //////////////////////////////////////////////////////////////////////////////
  //Set quadratic attenuation
  void C_Light::setQuadratic(const float aQuadratic)
  {
    quadratic = (float)aQuadratic;
  }
  //////////////////////////////////////////////////////////////////////////////
  //Set spotlight cutoff
  void C_Light::setCutoff(const float aCutoff)
  {
    cutoff = (float)aCutoff;
  }
  //////////////////////////////////////////////////////////////////////////////
  //Return light type
  int C_Light::getType()
  {
    return type;
  }
  //////////////////////////////////////////////////////////////////////////////
  //Return light position
  C_Vector3 C_Light::getPos()
  {
    return pos;
  }
  //////////////////////////////////////////////////////////////////////////////
  //Return light direction
  C_Vector3 C_Light::getDir()
  {
    return dir;
  }
  //////////////////////////////////////////////////////////////////////////////
  //Retun ambient color
  C_Vector3 C_Light::getAmbient()
  {
    return ambient;
  }
  //////////////////////////////////////////////////////////////////////////////
  //Return diffuse color
  C_Vector3 C_Light::getDiffuse()
  {
    return diffuse;
  }
  //////////////////////////////////////////////////////////////////////////////
  //Return specular color
  C_Vector3 C_Light::getSpecular()
  {
    return specular;
  }
  //////////////////////////////////////////////////////////////////////////////
  //Return constant attenuation
  float C_Light::getConstant()
  {
    return constant;
  }
  //////////////////////////////////////////////////////////////////////////////
  //Return linear attenuation
  float C_Light::getLinear()
  {
    return linear;
  }
  //////////////////////////////////////////////////////////////////////////////
  //Return quadratic attenuation
  float C_Light::getQuadratic()
  {
    return quadratic;
  }
  //////////////////////////////////////////////////////////////////////////////
  //Return spotlight cutoff
  float C_Light::getCutoff()
  {
    return cutoff;
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
