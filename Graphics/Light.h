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
    int mType = 0;

    C_Vector3 mPos = C_Vector3(0, 0, 5);
    C_Vector3 mDir = C_Vector3(-0.5, -0.4f, -0.3f);

    C_Vector3 mColor = C_Vector3(1, 1, 1);

    float mConstant = 1.0;
    float mLinear = 0.09;
    float mQuadratic = 0.032;

    float mInnerCutoff = 12.5;
    float mOuterCutoff = 17.5;
  public:
    //Constructor
    C_Light(const int aType);
    //Constructor 2
    C_Light(std::string aFile);
    //Set light type
    void setType(const int aType);
    //Set light position
    void setPos(const C_Vector3 aPos);
    //Set light direction
    void setDir(const C_Vector3 aDir);
    //Set light color
    void setColor(const C_Vector3 aColor);
    //Set constant attenuation
    void setConstant(const float aConst);
    //Set linear attenuation
    void setLinear(const float aLinear);
    //Set quadratic attenuation
    void setQuadratic(const float aQuadratic);
    //Set spotlight inner cutoff
    void setInnerCutoff(const float aInnerCutoff);
    //Set spotlight outer cuttof
    void setOuterCutoff(const float aOuterCutoff);

    //Return light type
    int getType();
    //Return light position
    C_Vector3 getPos() const;
    //Return light direction
    C_Vector3 getDir() const;
    //Return light color
    C_Vector3 getColor() const;
    //Return constant attenuation
    float getConstant() const;
    //Return linear attenuation
    float getLinear() const;
    //Return quadratic attenuation
    float getQuadratic() const;
    //Return spotlight inner cutoff
    float getInnerCutoff() const;
    //Return spotlight outer cutoff
    float getOuterCutoff() const;
    //Serialize to XML file
    void saveToFile(const char* aFile) const;
    //Deserialize from XML file
    void loadFromFile(const char* aFile);
    //Destructor
    ~C_Light();
  };

}
