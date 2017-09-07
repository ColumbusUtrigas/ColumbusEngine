/************************************************
*              ParticleEffect.cpp               *
*************************************************
*          This file is a part of:              *
*               COLUMBUS ENGINE                 *
*************************************************
*             Nikolay(Columbus) Red             *
*                   20.07.2017                  *
*************************************************/

#include <Graphics/ParticleEffect.h>

namespace C
{
  //////////////////////////////////////////////////////////////////////////////
  //Constructor
  C_ParticleEffect::C_ParticleEffect() {}
  //////////////////////////////////////////////////////////////////////////////
  //Set particle material
  void C_ParticleEffect::setMaterial(const C_Material* aMaterial)
  {
    mMaterial = (C_Material*)aMaterial;
  }
  //////////////////////////////////////////////////////////////////////////////
  //Set count of particles
  void C_ParticleEffect::setParticlesCount(const unsigned aParticlesCount)
  {
    mParticleCount = (unsigned)aParticlesCount;
  }
  //////////////////////////////////////////////////////////////////////////////
  //Set particles visible
  void C_ParticleEffect::setVisible(const bool aVisible)
  {
    mVisible = (bool)aVisible;
  }
  //////////////////////////////////////////////////////////////////////////////
  //Set particles scale over lifetime
  void C_ParticleEffect::setScaleOverLifetime(const bool aA)
  {
	  mScaleOverLifetime = (bool)aA;
  }
  //////////////////////////////////////////////////////////////////////////////
  //Set particles emit from shell
  void C_ParticleEffect::setEmitFromShell(const bool aA)
  {
    mEmitFromShell = (bool)aA;
  }
  //////////////////////////////////////////////////////////////////////////////
  //Set particles additive blending
  void C_ParticleEffect::setAdditive(const bool aA)
  {
    mAdditive = (bool)aA;
  }
  //////////////////////////////////////////////////////////////////////////////
  //Set negative direction limit
  void C_ParticleEffect::setMinDirection(const C_Vector3 aMinDirection)
  {
    mMinDirection = (C_Vector3)aMinDirection;
  }
  //////////////////////////////////////////////////////////////////////////////
  //Set positive direction limit
  void C_ParticleEffect::setMaxDirection(const C_Vector3 aMaxDirection)
  {
    mMaxDirection = (C_Vector3)aMaxDirection;
  }
  //////////////////////////////////////////////////////////////////////////////
  //Set constant force acting on particles
  void C_ParticleEffect::setConstantForce(const C_Vector3 aConstantForce)
  {
    mConstantForce = (C_Vector3)aConstantForce;
  }
  //////////////////////////////////////////////////////////////////////////////
  //Set particle size
  void C_ParticleEffect::setParticleSize(const C_Vector2 aParticleSize)
  {
    mParticleSize = (C_Vector2)aParticleSize;
  }
  //////////////////////////////////////////////////////////////////////////////
  //Set particle start size
  void C_ParticleEffect::setStartSize(C_Vector2 aStartSize)
  {
	  mStartSize = (C_Vector2)aStartSize;
  }
  //////////////////////////////////////////////////////////////////////////////
  //Set particle final size
  void C_ParticleEffect::setFinalSize(C_Vector2 aFinalSize)
  {
	  mFinalSize = (C_Vector2)aFinalSize;
  }
  //////////////////////////////////////////////////////////////////////////////
  //Set particle start color
  void C_ParticleEffect::setStartColor(C_Vector4 aStartColor)
  {
	  mStartColor = (C_Vector4)aStartColor;
  }
  //////////////////////////////////////////////////////////////////////////////
  //Set particle final color
  void C_ParticleEffect::setFinalColor(C_Vector4 aFinalColor)
  {
	  mFinalColor = (C_Vector4)aFinalColor;
  }
  //////////////////////////////////////////////////////////////////////////////
  //Set paritcle minimum time to live
  void C_ParticleEffect::setMinTimeToLive(const float aMinTimeToLive)
  {
    mMinTimeToLive = (float)aMinTimeToLive;
  }
  //////////////////////////////////////////////////////////////////////////////
  //Set particle maximum time to live
  void C_ParticleEffect::setMaxTimeToLive(const float aMaxTimeToLive)
  {
    mMaxTimeToLive = (float)aMaxTimeToLive;
  }
  //////////////////////////////////////////////////////////////////////////////
  //Set particle minimum velocity
  void C_ParticleEffect::setMinVelocity(const float aMinVelocity)
  {
    mMinVelocity = (float)aMinVelocity;
  }
  //////////////////////////////////////////////////////////////////////////////
  //Set particle maximum velocity
  void C_ParticleEffect::setMaxVelocity(const float aMaxVelocity)
  {
    mMaxVelocity = (float)aMaxVelocity;
  }
  //////////////////////////////////////////////////////////////////////////////
  //Set particle minimum roation
  void C_ParticleEffect::setMinRotation(const float aMinRotation)
  {
    mMinRotation = (float)aMinRotation;
  }
  //////////////////////////////////////////////////////////////////////////////
  //Set particle maximum rotation
  void C_ParticleEffect::setMaxRotation(const float aMaxRotation)
  {
    mMaxRotation = (float)aMaxRotation;
  }
  //////////////////////////////////////////////////////////////////////////////
  //Set particle minimum rotation speed
  void C_ParticleEffect::setMinRotationSpeed(const float aMinRotationSpeed)
  {
    mMinRotationSpeed = (float)aMinRotationSpeed;
  }
  //////////////////////////////////////////////////////////////////////////////
  //Set particle maximum rotation speed
  void C_ParticleEffect::setMaxRotationSpeed(const float aMaxRotationSpeed)
  {
    mMaxRotationSpeed = (float)aMaxRotationSpeed;
  }
  //////////////////////////////////////////////////////////////////////////////
  //Set particles emit rate
  void C_ParticleEffect::setEmitRate(const int aEmitRate)
  {
    mEmitRate = (int)aEmitRate;
  }
  //////////////////////////////////////////////////////////////////////////////
  //Set particle shape
  void C_ParticleEffect::setParticleShape(const int aParticleShape)
  {
    mParticleShape = (int)aParticleShape;
  }
  //////////////////////////////////////////////////////////////////////////////
  void C_ParticleEffect::setParticleShapeRadius(const float aRadius)
  {
    mParticleShapeRadius = (float)aRadius;
  }
  //////////////////////////////////////////////////////////////////////////////
  //Return material
  C_Material* C_ParticleEffect::getMaterial()
  {
    return mMaterial;
  }
  //////////////////////////////////////////////////////////////////////////////
  //Return particles count
  unsigned C_ParticleEffect::getParticlesCount()
  {
    return mParticleCount;
  }
  //////////////////////////////////////////////////////////////////////////////
  //Return particles visible
  bool C_ParticleEffect::getVisible()
  {
    return mVisible;
  }
  //////////////////////////////////////////////////////////////////////////////
  //Return particles scale over lifetime
  bool C_ParticleEffect::getScaleOverLifetime()
  {
	  return mScaleOverLifetime;
  }
  //////////////////////////////////////////////////////////////////////////////
  //Return particles emit from shell
  bool C_ParticleEffect::getEmitFromShell()
  {
    return mEmitFromShell;
  }
  //////////////////////////////////////////////////////////////////////////////
  //Return particles additive blending
  bool C_ParticleEffect::getAdditive()
  {
    return mAdditive;
  }
  //////////////////////////////////////////////////////////////////////////////
  //Return minimum particle direction
  C_Vector3 C_ParticleEffect::getMinDirection()
  {
    return mMinDirection;
  }
  //////////////////////////////////////////////////////////////////////////////
  //Return maximum particle direction
  C_Vector3 C_ParticleEffect::getMaxDirection()
  {
    return mMaxDirection;
  }
  //////////////////////////////////////////////////////////////////////////////
  //Return constant force acting on particles
  C_Vector3 C_ParticleEffect::getConstantForce()
  {
    return mConstantForce;
  }
  //////////////////////////////////////////////////////////////////////////////
  //Return particle size
  C_Vector2 C_ParticleEffect::getParticleSize()
  {
    return mParticleSize;
  }
  //////////////////////////////////////////////////////////////////////////////
  //Return particle start size
  C_Vector2 C_ParticleEffect::getStartSize()
  {
	  return mStartSize;
  }
  //////////////////////////////////////////////////////////////////////////////
  //Return particle final size
  C_Vector2 C_ParticleEffect::getFinalSize()
  {
	  return mFinalSize;
  }
  //////////////////////////////////////////////////////////////////////////////
  //Return particle start color
  C_Vector4 C_ParticleEffect::getStartColor()
  {
	  return mStartColor;;
  }
  //////////////////////////////////////////////////////////////////////////////
  //Return particle final color
  C_Vector4 C_ParticleEffect::getFinalColor()
  {
	  return mFinalColor;
  }
  //////////////////////////////////////////////////////////////////////////////
  //Return particle minimum time to live
  float C_ParticleEffect::getMinTimeToLive()
  {
    return mMinTimeToLive;
  }
  //////////////////////////////////////////////////////////////////////////////
  //Return particle maximum time to live
  float C_ParticleEffect::getMaxTimeToLive()
  {
    return mMaxTimeToLive;
  }
  //////////////////////////////////////////////////////////////////////////////
  //Return particle minimum velocity
  float C_ParticleEffect::getMinVelocity()
  {
    return mMinVelocity;
  }
  //////////////////////////////////////////////////////////////////////////////
  //Return particle maximum velocity
  float C_ParticleEffect::getMaxVelocity()
  {
    return mMaxVelocity;
  }
  //////////////////////////////////////////////////////////////////////////////
  //Return particle minimum rotation
  float C_ParticleEffect::getMinRotation()
  {
    return mMinRotation;
  }
  //////////////////////////////////////////////////////////////////////////////
  //Return particle maximum rotation
  float C_ParticleEffect::getMaxRotation()
  {
    return mMaxRotation;
  }
  //////////////////////////////////////////////////////////////////////////////
  //Return particle minimum rotation speed
  float C_ParticleEffect::getMinRotationSpeed()
  {
    return mMinRotationSpeed;
  }
  //////////////////////////////////////////////////////////////////////////////
  //Return particle maximum rotation speed
  float C_ParticleEffect::getMaxRotationSpeed()
  {
    return mMaxRotationSpeed;
  }
  //////////////////////////////////////////////////////////////////////////////
  //Return particles emit rate
  int C_ParticleEffect::getEmitRate()
  {
    return mEmitRate;
  }
  //////////////////////////////////////////////////////////////////////////////
  //Retun particle shape
  int C_ParticleEffect::getParticleShape()
  {
    return mParticleShape;
  }
  //Return particle shape radius
  float C_ParticleEffect::getParticleShapeRadius()
  {
    return mParticleShapeRadius;
  }
  //////////////////////////////////////////////////////////////////////////////
  //Serialize to XML file
  void C_ParticleEffect::saveToXLM(const char* aFile)
  {
	  C_XMLDoc doc;
	  C_XMLNode* root = doc.NewElement("ParticleEffect");
	  doc.InsertFirstChild(root);

	  C_XMLElement* tmp = doc.NewElement("Count");
	  if (tmp == NULL && tmp == nullptr)
	  {
		  C_Error("Can't save Particles count: %s", aFile);
		  return;
	  }
	  tmp->SetText(mParticleCount);
	  root->InsertEndChild(tmp);

	  tmp = NULL;

	  tmp = doc.NewElement("Visible");
	  if (tmp == NULL && tmp == nullptr)
	  {
		  C_Error("Can't save Particles visible: %s", aFile);
		  return;
	  }
	  tmp->SetText(mVisible);
	  root->InsertEndChild(tmp);

	  tmp = NULL;

	  tmp = doc.NewElement("ScaleOL");
	  if (tmp == NULL && tmp == nullptr)
	  {
		  C_Error("Can't save Particles scale over lifetime: %s", aFile);
		  return;
	  }
	  tmp->SetText(mScaleOverLifetime);
	  root->InsertEndChild(tmp);

	  tmp = NULL;

    tmp = doc.NewElement("EmitFromShell");
	  if (tmp == NULL && tmp == nullptr)
	  {
		  C_Error("Can't save Particles emit from shell: %s", aFile);
		  return;
	  }
	  tmp->SetText(mEmitFromShell);
	  root->InsertEndChild(tmp);

	  tmp = NULL;

    tmp = doc.NewElement("AdditiveBlending");
	  if (tmp == NULL && tmp == nullptr)
	  {
		  C_Error("Can't save Particles additive blending: %s", aFile);
		  return;
	  }
	  tmp->SetText(mAdditive);
	  root->InsertEndChild(tmp);

	  tmp = NULL;

	  tmp = doc.NewElement("MinDirection");
	  if (tmp == NULL && tmp == nullptr)
	  {
		  C_Error("Can't save Particles min direction: %s", aFile);
		  return;
	  }
	  tmp->SetAttribute("X", mMinDirection.x);
	  tmp->SetAttribute("Y", mMinDirection.y);
	  tmp->SetAttribute("Z", mMinDirection.z);
	  root->InsertEndChild(tmp);

	  tmp = NULL;

	  tmp = doc.NewElement("MaxDirection");
	  if (tmp == NULL && tmp == nullptr)
	  {
		  C_Error("Can't save Particles max direction: %s", aFile);
		  return;
	  }
	  tmp->SetAttribute("X", mMaxDirection.x);
	  tmp->SetAttribute("Y", mMaxDirection.y);
	  tmp->SetAttribute("Z", mMaxDirection.z);
	  root->InsertEndChild(tmp);

	  tmp = NULL;

	  tmp = doc.NewElement("ConstForce");
	  if (tmp == NULL && tmp == nullptr)
	  {
		  C_Error("Can't save Particles constant force: %s", aFile);
		  return;
	  }
	  tmp->SetAttribute("X", mConstantForce.x);
	  tmp->SetAttribute("Y", mConstantForce.y);
	  tmp->SetAttribute("Z", mConstantForce.z);
	  root->InsertEndChild(tmp);

	  tmp = NULL;

	  tmp = doc.NewElement("PartSize");
	  if (tmp == NULL && tmp == nullptr)
	  {
		  C_Error("Can't save Particles size: %s", aFile);
		  return;
	  }
	  tmp->SetAttribute("X", mParticleSize.x);
	  tmp->SetAttribute("Y", mParticleSize.y);
	  root->InsertEndChild(tmp);

	  tmp = NULL;

	  tmp = doc.NewElement("StartSize");
	  if (tmp == NULL && tmp == nullptr)
	  {
		  C_Error("Can't save Particles start size: %s", aFile);
		  return;
	  }
	  tmp->SetAttribute("X", mStartSize.x);
	  tmp->SetAttribute("Y", mStartSize.y);
	  root->InsertEndChild(tmp);

	  tmp = NULL;

	  tmp = doc.NewElement("FinalSize");
	  if (tmp == NULL && tmp == nullptr)
	  {
		  C_Error("Can't save Particles final size: %s", aFile);
		  return;
	  }
	  tmp->SetAttribute("X", mFinalSize.x);
	  tmp->SetAttribute("Y", mFinalSize.y);
	  root->InsertEndChild(tmp);

	  tmp = NULL;

	  tmp = doc.NewElement("StartColor");
	  if (tmp == NULL && tmp == nullptr)
	  {
		  C_Error("Can't save Particles start color: %s", aFile);
		  return;
	  }
	  tmp->SetAttribute("R", mStartColor.x);
	  tmp->SetAttribute("G", mStartColor.y);
	  tmp->SetAttribute("B", mStartColor.z);
	  tmp->SetAttribute("A", mStartColor.w);
	  root->InsertEndChild(tmp);

	  tmp = NULL;

	  tmp = doc.NewElement("FinalColor");
	  if (tmp == NULL && tmp == nullptr)
	  {
		  C_Error("Can't save Particles final color: %s", aFile);
		  return;
	  }
	  tmp->SetAttribute("R", mFinalColor.x);
	  tmp->SetAttribute("G", mFinalColor.y);
	  tmp->SetAttribute("B", mFinalColor.z);
	  tmp->SetAttribute("A", mFinalColor.w);
	  root->InsertEndChild(tmp);

	  tmp = NULL;

	  tmp = doc.NewElement("MinTTL");
	  if (tmp == NULL && tmp == nullptr)
	  {
		  C_Error("Can't save Particles min TTL: %s", aFile);
		  return;
	  }
	  tmp->SetText(mMinTimeToLive);
	  root->InsertEndChild(tmp);
	  tmp = NULL;

	  tmp = doc.NewElement("MaxTTL");
	  if (tmp == NULL && tmp == nullptr)
	  {
		  C_Error("Can't save Particles max TTL: %s", aFile);
		  return;
	  }
	  tmp->SetText(mMaxTimeToLive);
	  root->InsertEndChild(tmp);
	  tmp = NULL;

	  tmp = doc.NewElement("MinVelocity");
	  if (tmp == NULL && tmp == nullptr)
	  {
		  C_Error("Can't save Particles min velocity: %s", aFile);
		  return;
	  }
	  tmp->SetText(mMinVelocity);
	  root->InsertEndChild(tmp);
	  tmp = NULL;

	  tmp = doc.NewElement("MaxVelocity");
	  if (tmp == NULL && tmp == nullptr)
	  {
		  C_Error("Can't save Particles max velocity: %s", aFile);
		  return;
	  }
	  tmp->SetText(mMaxVelocity);
	  root->InsertEndChild(tmp);
	  tmp = NULL;

	  tmp = doc.NewElement("MinRotation");
	  if (tmp == NULL && tmp == nullptr)
	  {
		  C_Error("Can't save Particles min rotation: %s", aFile);
		  return;
	  }
	  tmp->SetText(mMinRotation);
	  root->InsertEndChild(tmp);
	  tmp = NULL;

	  tmp = doc.NewElement("MaxRotation");
	  if (tmp == NULL && tmp == nullptr)
	  {
		  C_Error("Can't save Particles max rotation: %s", aFile);
		  return;
	  }
	  tmp->SetText(mMaxRotation);
	  root->InsertEndChild(tmp);
	  tmp = NULL;

	  tmp = doc.NewElement("MinRotationSpeed");
	  if (tmp == NULL && tmp == nullptr)
	  {
		  C_Error("Can't save Particles min rotation speed: %s", aFile);
		  return;
	  }
	  tmp->SetText(mMinRotationSpeed);
	  root->InsertEndChild(tmp);
	  tmp = NULL;

	  tmp = doc.NewElement("MaxRotationSpeed");
	  if (tmp == NULL && tmp == nullptr)
	  {
		  C_Error("Can't save Particles max rotation speed: %s", aFile);
		  return;
	  }
	  tmp->SetText(mMaxRotationSpeed);
	  root->InsertEndChild(tmp);
	  tmp = NULL;

    tmp = doc.NewElement("EmitRate");
	  if (tmp == NULL && tmp == nullptr)
    {
		  C_Error("Can't save Particles emit rate: %s", aFile);
		  return;
	  }
	  tmp->SetText(mEmitRate);
	  root->InsertEndChild(tmp);
	  tmp = NULL;

    tmp = doc.NewElement("Shape");
	  if (tmp == NULL && tmp == nullptr)
	  {
		  C_Error("Can't save Particles shape: %s", aFile);
		  return;
	  }
	  tmp->SetText(mParticleShape);
	  root->InsertEndChild(tmp);
	  tmp = NULL;

    tmp = doc.NewElement("ShapeRadius");
	  if (tmp == NULL && tmp == nullptr)
	  {
		  C_Error("Can't save Particles shape radius: %s", aFile);
		  return;
	  }
	  tmp->SetText(mParticleShapeRadius);
	  root->InsertEndChild(tmp);
	  tmp = NULL;

	  if (doc.SaveFile(aFile) != C_XML_SUCCESS)
	  {
		  C_Error("Can't save Particle Effect: %s", aFile);
		  return;
	  }

	  C_Success("Particle Effect saved: %s", aFile);
  }
  //////////////////////////////////////////////////////////////////////////////
  //Serialize from XML file
  void C_ParticleEffect::loadFromXML(const char* aFile)
  {
	  C_XMLDoc doc;
	  if (doc.LoadFile(aFile) != C_XML_SUCCESS)
	  {
		  C_Error("Can't load Particle Effect: %s\n", aFile);
		  return;
	  }

	  C_XMLElement* root = doc.FirstChildElement("ParticleEffect");
	  if (root == NULL && root == nullptr)
	  {
		  C_Error("Can't load Particle Effect: %s\n", aFile);
		  return;
	  }

	  C_XMLElement* tmp = root->FirstChildElement("Count");
	  if (tmp == NULL && tmp == nullptr)
	  {
		  C_Error("Can't load Particles count: %s", aFile);
		  return;
	  }

	  tmp->QueryIntText((int*)&mParticleCount);

	  tmp = NULL;

	  tmp = root->FirstChildElement("Visible");
	  if (tmp == NULL && tmp == nullptr)
	  {
		  C_Error("Can't load Particles visible: %s", aFile);
		  return;
	  }

	  tmp->QueryBoolText(&mVisible);

	  tmp = NULL;

	  tmp = root->FirstChildElement("ScaleOL");
	  if (tmp == NULL && tmp == nullptr)
	  {
		  C_Error("Can't load Particles scale over lifetime: %s", aFile);
		  return;
	  }

	  tmp->QueryBoolText(&mScaleOverLifetime);

	  tmp = NULL;

    tmp = root->FirstChildElement("EmitFromShell");
	  if (tmp == NULL && tmp == nullptr)
	  {
		  C_Error("Can't load Particles emit from shell: %s", aFile);
		  return;
	  }

	  tmp->QueryBoolText(&mEmitFromShell);

	  tmp = NULL;

    tmp = root->FirstChildElement("AdditiveBlending");
	  if (tmp == NULL && tmp == nullptr)
	  {
		  C_Error("Can't load Particles additive blending: %s", aFile);
		  return;
	  }

	  tmp->QueryBoolText(&mAdditive);

	  tmp = NULL;

	  tmp = root->FirstChildElement("MinDirection");
	  if (tmp == NULL && tmp == nullptr)
	  {
		  C_Error("Can't load Particles min direction: %s", aFile);
		  return;
	  }

	  tmp->QueryAttribute("X", &mMinDirection.x);
	  tmp->QueryAttribute("Y", &mMinDirection.y);
	  tmp->QueryAttribute("Z", &mMinDirection.z);

	  tmp = NULL;

	  tmp = root->FirstChildElement("MaxDirection");
	  if (tmp == NULL && tmp == nullptr)
	  {
		  C_Error("Can't load Particles max direction: %s", aFile);
		  return;
	  }

	  tmp->QueryAttribute("X", &mMaxDirection.x);
	  tmp->QueryAttribute("Y", &mMaxDirection.y);
	  tmp->QueryAttribute("Z", &mMaxDirection.z);

	  tmp = NULL;

	  tmp = root->FirstChildElement("ConstForce");
	  if (tmp == NULL && tmp == nullptr)
	  {
		  C_Error("Can't load Particles constant force: %s", aFile);
		  return;
	  }

	  tmp->QueryAttribute("X", &mConstantForce.x);
	  tmp->QueryAttribute("Y", &mConstantForce.y);
	  tmp->QueryAttribute("Z", &mConstantForce.z);

	  tmp = NULL;

	  tmp = root->FirstChildElement("PartSize");
	  if (tmp == NULL && tmp == nullptr)
	  {
		  C_Error("Can't load Particles size: %s", aFile);
		  return;
	  }

	  tmp->QueryAttribute("X", &mParticleSize.x);
	  tmp->QueryAttribute("Y", &mParticleSize.y);

	  tmp = NULL;

	  tmp = root->FirstChildElement("StartSize");
	  if (tmp == NULL && tmp == nullptr)
	  {
		  C_Error("Can't load Particles start size: %s", aFile);
		  return;
	  }

	  tmp->QueryAttribute("X", &mStartSize.x);
	  tmp->QueryAttribute("Y", &mStartSize.y);

	  tmp = NULL;

	  tmp = root->FirstChildElement("FinalSize");
	  if (tmp == NULL && tmp == nullptr)
	  {
		  C_Error("Can't load Particles final size: %s", aFile);
		  return;
	  }

	  tmp->QueryAttribute("X", &mFinalSize.x);
	  tmp->QueryAttribute("Y", &mFinalSize.y);

	  tmp = NULL;

	  tmp = root->FirstChildElement("StartColor");
	  if (tmp == NULL && tmp == nullptr)
	  {
		  C_Error("Can't load Particles start color: %s", aFile);
		  return;
	  }

	  tmp->QueryAttribute("R", &mStartColor.x);
	  tmp->QueryAttribute("G", &mStartColor.y);
	  tmp->QueryAttribute("B", &mStartColor.z);
	  tmp->QueryAttribute("A", &mStartColor.w);

	  tmp = NULL;

	  tmp = root->FirstChildElement("FinalColor");
	  if (tmp == NULL && tmp == nullptr)
	  {
		  C_Error("Can't load Particles final color: %s", aFile);
		  return;
	  }

	  tmp->QueryAttribute("R", &mFinalColor.x);
	  tmp->QueryAttribute("G", &mFinalColor.y);
	  tmp->QueryAttribute("B", &mFinalColor.z);
	  tmp->QueryAttribute("A", &mFinalColor.w);

	  tmp = NULL;

	  tmp = root->FirstChildElement("MinTTL");
	  if (tmp == NULL && tmp == nullptr)
	  {
		  C_Error("Can't load Particles min TTL: %s", aFile);
		  return;
	  }
	  tmp->QueryFloatText(&mMinTimeToLive);
	  tmp = NULL;

	  tmp = root->FirstChildElement("MaxTTL");
	  if (tmp == NULL && tmp == nullptr)
	  {
		  C_Error("Can't load Particles max TTL: %s", aFile);
		  return;
	  }
	  tmp->QueryFloatText(&mMaxTimeToLive);
	  tmp = NULL;

	  tmp = root->FirstChildElement("MinVelocity");
	  if (tmp == NULL && tmp == nullptr)
	  {
		  C_Error("Can't load Particles min velocity: %s", aFile);
		  return;
	  }
	  tmp->QueryFloatText(&mMinVelocity);
	  tmp = NULL;

	  tmp = root->FirstChildElement("MaxVelocity");
	  if (tmp == NULL && tmp == nullptr)
	  {
		  C_Error("Can't load Particles max velocity: %s", aFile);
		  return;
	  }
	  tmp->QueryFloatText(&mMaxVelocity);
	  tmp = NULL;

	  tmp = root->FirstChildElement("MinRotation");
	  if (tmp == NULL && tmp == nullptr)
	  {
		  C_Error("Can't load Particles min rotation: %s", aFile);
		  return;
	  }
	  tmp->QueryFloatText(&mMinRotation);
	  tmp = NULL;

	  tmp = root->FirstChildElement("MaxRotation");
	  if (tmp == NULL && tmp == nullptr)
	  {
		  C_Error("Can't load Particles max rotation: %s", aFile);
		  return;
	  }
	  tmp->QueryFloatText(&mMaxRotation);
	  tmp = NULL;

	  tmp = root->FirstChildElement("MinRotationSpeed");
	  if (tmp == NULL && tmp == nullptr)
	  {
		  C_Error("Can't load Particles min rotation speed: %s", aFile);
		  return;
	  }
	  tmp->QueryFloatText(&mMinRotationSpeed);
	  tmp = NULL;

	  tmp = root->FirstChildElement("MaxRotationSpeed");
	  if (tmp == NULL && tmp == nullptr)
	  {
		  C_Error("Can't load Particles max rotation speed: %s", aFile);
		  return;
	  }
	  tmp->QueryFloatText(&mMaxRotationSpeed);
	  tmp = NULL;

    tmp = root->FirstChildElement("EmitRate");
	  if (tmp == NULL && tmp == nullptr)
	  {
		  C_Error("Can't load Particles emit rate: %s", aFile);
		  return;
	  }
	  tmp->QueryIntText(&mEmitRate);
	  tmp = NULL;

    tmp = root->FirstChildElement("Shape");
	  if (tmp == NULL && tmp == nullptr)
	  {
		  C_Error("Can't load Particles shape: %s", aFile);
		  return;
	  }
	  tmp->QueryIntText(&mParticleShape);
	  tmp = NULL;

    tmp = root->FirstChildElement("ShapeRadius");
	  if (tmp == NULL && tmp == nullptr)
	  {
		  C_Error("Can't load Particles shape radius: %s", aFile);
		  return;
	  }
	  tmp->QueryFloatText(&mParticleShapeRadius);
	  tmp = NULL;

	  C_Success("Particle Effect loaded: %s", aFile);
  }
  //////////////////////////////////////////////////////////////////////////////
  //Destructor
  C_ParticleEffect::~C_ParticleEffect() {}

}
