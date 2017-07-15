#pragma once
#ifndef LIGHT_H
#define LIGHT_H

#include <System/System.h>
#include <Math/Vector2.h>
#include <Math/Vector3.h>
#include <Math/Vector4.h>

namespace C
{

  class C_Light
  {
  private:
    int type;

    C_Vector3 pos;
    C_Vector3 dir;

    C_Vector3 ambient;
    C_Vector3 diffuse;
    C_Vector3 specular;

    float constant;
    float linear;
    float quadratic;

    float cutoff;
  public:
    C_Light(const int aType);
    C_Light(const char* aFile);

    void setType(const int aType);

    void setPos(const C_Vector3 aPos);

    void setDir(const C_Vector3 aDir);

    void setAmbient(const C_Vector3 aAmbient);

    void setDiffuse(const C_Vector3 aDiffuse);

    void setSpecular(const C_Vector3 aSpecular);

    void setConstant(const float aConst);

    void setLinear(const float aLinear);

    void setQuadratic(const float aQuadratic);

    void setCutoff(const float aCutoff);

    int getType();

    C_Vector3 getPos();

    C_Vector3 getDir();

    C_Vector3 getAmbient();

    C_Vector3 getDiffuse();

    C_Vector3 getSpecular();

    float getConstant();

    float getLinear();

    float getQuadratic();

    float getCutoff();

    void saveToFile(const char* aFile);

    void loadFromFile(const char* aFile);

    ~C_Light();
  };

}

#endif /* end of include guard: LIGHT_H */
