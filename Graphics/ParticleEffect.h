/************************************************
*              ParticleEffect.h                 *
*************************************************
*          This file is a part of:              *
*               COLUMBUS ENGINE                 *
*************************************************
*             Nikolay(Columbus) Red             *
*                   20.07.2017                  *
*************************************************/

#pragma once

#include <Graphics/Material.h>

namespace C
{

  class C_ParticleEffect
  {
  private:
    C_Material* mMaterial = NULL;

    unsigned mParticleCount = 5;

    bool mVisible = true;

    C_Vector3 mMinDirection = C_Vector3(-1, -1, -1);
    C_Vector3 mMaxDirection = C_Vector3(1, 1, 1);
    C_Vector3 mConstantForce = C_Vector3(0, 0, 0);

    C_Vector2 mParticleSize = C_Vector2(1, 1);

    float mMinTimeToLive = 1.0;
    float mMaxTimeToLive = 1.0;
    float mMinVelocity = 1.0;
    float mMaxVelocity = 1.0;
    float mMinRotation = 0.0;
    float mMaxRotation = 0.0;
    float mMinRotationSpeed = 0.0;
    float mMaxRotationSpeed = 0.0;
  public:
    //Constructor
    C_ParticleEffect();
    //Set particles material
    void setMaterial(const C_Material* aMaterial);
    //Set count of particles
    void setParticlesCount(const unsigned aParticlesCount);
    //Set particles visible
    void setVisible(const bool aVisible);
    //Set negative direction limit
    void setMinDirection(const C_Vector3 aMinDirection);
    //Set positive direction limit
    void setMaxDirection(const C_Vector3 aMaxDirection);
    //Set constant force acting on particles
    void setConstantForce(const C_Vector3 aConstantForce);
    //Set particle size
    void setParticleSize(const C_Vector2 aParticleSize);
    //Set particle minimum time to live
    void setMinTimeToLive(const float aMinTimeToLive);
    //Set particle maximum time to live
    void setMaxTimeToLive(const float aMaxTimeToLive);
    //Set particle minimum velocity
    void setMinVelocity(const float aMinVelocity);
    //Set particle maximum velocity
    void setMaxVelocity(const float aMaxVelocity);
    //Set particle minimum rotation
    void setMinRotation(const float aMinRotation);
    //Set particle maximum rotation
    void setMaxRotation(const float aMaxRotation);
    //Set particle minimum rotation speed
    void setMinRotationSpeed(const float aMinRotationSpeed);
    //Set particle maximum rotation speed
    void setMaxRotationSpeed(const float aMaxRotationSpeed);


    //Return material
    C_Material* getMaterial();
    //Return particles count
    unsigned getParticlesCount();
    //Return particles visible
    bool getVisible();
    //Return minimum particle direction
    C_Vector3 getMinDirection();
    //Return maximum particle direction
    C_Vector3 getMaxDirection();
    //Return constant force acting on particles
    C_Vector3 getConstantForce();
    //Return particle size
    C_Vector2 getParticleSize();
    //Return particle minimum time to live
    float getMinTimeToLive();
    //Return particle maximum time to live
    float getMaxTimeToLive();
    //Return particle minimum velocity
    float getMinVelocity();
    //Return particle maximum velocity
    float getMaxVelocity();
    //Return particle minimum rotation
    float getMinRotation();
    //Return partile maximum rotation
    float getMaxRotation();
    //Return particle minimum rotation speed
    float getMinRotationSpeed();
    //Return particle maximum rotation speed
    float getMaxRotationSpeed();
    //Destructor
    ~C_ParticleEffect();
  };

}
