#include <Graphics/Light.h>

namespace C
{

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

  void C_Light::setType(const int aType)
  {
    type = (int)aType;
  }

  void C_Light::setPos(const C_Vector3 aPos)
  {
    pos = (C_Vector3)aPos;
  }

  void C_Light::setDir(const C_Vector3 aDir)
  {
    dir = (C_Vector3)aDir;
  }

  void C_Light::setAmbient(const C_Vector3 aAmbient)
  {
    ambient = (C_Vector3)aAmbient;
  }

  void C_Light::setDiffuse(const C_Vector3 aDiffuse)
  {
    diffuse = (C_Vector3)aDiffuse;
  }

  void C_Light::setSpecular(const C_Vector3 aSpecular)
  {
    specular = (C_Vector3)aSpecular;
  }

  void C_Light::setConstant(const float aConstant)
  {
    constant = (float)aConstant;
  }

  void C_Light::setLinear(const float aLinear)
  {
    linear = (float)aLinear;
  }

  void C_Light::setQuadratic(const float aQuadratic)
  {
    quadratic = (float)aQuadratic;
  }

  void C_Light::setCutoff(const float aCutoff)
  {
    cutoff = (float)aCutoff;
  }

  int C_Light::getType()
  {
    return type;
  }

  C_Vector3 C_Light::getPos()
  {
    return pos;
  }

  C_Vector3 C_Light::getDir()
  {
    return dir;
  }

  C_Vector3 C_Light::getAmbient()
  {
    return ambient;
  }

  C_Vector3 C_Light::getDiffuse()
  {
    return diffuse;
  }

  C_Vector3 C_Light::getSpecular()
  {
    return specular;
  }

  float C_Light::getConstant()
  {
    return constant;
  }

  float C_Light::getLinear()
  {
    return linear;
  }

  float C_Light::getQuadratic()
  {
    return quadratic;
  }

  float C_Light::getCutoff()
  {
    return cutoff;
  }

  void C_Light::saveToFile(const char* aFile)
  {

  }

  void C_Light::loadFromFile(const char* aFile)
  {

  }

  C_Light::~C_Light()
  {

  }

}
