/************************************************
*                   Light.h                     *
*************************************************
*          This file is a part of:              *
*               COLUMBUS ENGINE                 *
*************************************************
*             Nikolay(Columbus) Red             *
*                   20.07.2017                  *
*************************************************/

#pragma once

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
    float outerCutoff;
  public:
    //Constructor
    C_Light(const int aType);
    //Constructor 2
    C_Light(const char* aFile);
    //Set light type
    void setType(const int aType);
    //Set light position
    void setPos(const C_Vector3 aPos);
    //Set light direction
    void setDir(const C_Vector3 aDir);
    //Set ambient color
    void setAmbient(const C_Vector3 aAmbient);
    //Set diffuse color
    void setDiffuse(const C_Vector3 aDiffuse);
    //Set specular color
    void setSpecular(const C_Vector3 aSpecular);
    //Set constant attenuation
    void setConstant(const float aConst);
    //Set linear attenuation
    void setLinear(const float aLinear);
    //Set quadratic attenuation
    void setQuadratic(const float aQuadratic);
    //Set spotlight cutoff
    void setCutoff(const float aCutoff);
    //Set spotlight outer cuttof
    void setOuterCutoff(const float aOuterCutoff);
    //Return light type
    int getType();
    //Return light position
    C_Vector3 getPos();
    //Return light direction
    C_Vector3 getDir();
    //Return ambient color
    C_Vector3 getAmbient();
    //Return diffuse color
    C_Vector3 getDiffuse();
    //Return specular color
    C_Vector3 getSpecular();
    //Return constant attenuation
    float getConstant();
    //Return linear attenuation
    float getLinear();
    //Return quadratic attenuation
    float getQuadratic();
    //Return spotlight cutoff
    float getCutoff();
    //Return spotlight outer cutoff
    float getOuterCutoff();
    //Serialize to XML file
    void saveToFile(const char* aFile);
    //Deserialize from XML file
    void loadFromFile(const char* aFile);
    //Destructor
    ~C_Light();
  };

}
