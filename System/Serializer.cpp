/************************************************
*              	 Serializer.cpp                 *
*************************************************
*          This file is a part of:              *
*               COLUMBUS ENGINE                 *
*************************************************
*             Nikolay(Columbus) Red             *
*                   18.10.2017                  *
*************************************************/

#include <System/Serializer.h>

namespace C
{

  namespace Serializer
  {

    //////////////////////////////////////////////////////////////////////////////
    C_SerializerXML::C_SerializerXML(std::string aFile, std::string aRoot)
    {
      mFile = aFile;
      mRootName = aRoot;
      mRoot = mDoc.NewElement(aRoot.c_str());
      mDoc.InsertFirstChild(mRoot);
    }
    //////////////////////////////////////////////////////////////////////////////
    bool C_SerializerXML::setInt(std::string aElement, int aValue)
    {
      if (mMode != 0) return false;

      mTmp = mDoc.NewElement(aElement.c_str());
      if (mTmp == nullptr) return false;
      mTmp->SetText(aValue);
      mRoot->InsertEndChild(mTmp);
      mTmp = nullptr;
      return true;
    }
    //////////////////////////////////////////////////////////////////////////////
    bool C_SerializerXML::setBool(std::string aElement, bool aValue)
    {
      if (mMode != 0) return false;

      mTmp = mDoc.NewElement(aElement.c_str());
      if (mTmp == nullptr) return false;
      mTmp->SetText(aValue);
      mRoot->InsertEndChild(mTmp);
      mTmp = nullptr;
      return true;
    }
    //////////////////////////////////////////////////////////////////////////////
    bool C_SerializerXML::setFloat(std::string aElement, float aValue)
    {
      if (mMode != 0) return false;

      mTmp = mDoc.NewElement(aElement.c_str());
      if (mTmp == nullptr) return false;
      mTmp->SetText(aValue);
      mRoot->InsertEndChild(mTmp);
      mTmp = nullptr;
      return true;
    }
    //////////////////////////////////////////////////////////////////////////////
    bool C_SerializerXML::setDouble(std::string aElement, double aValue)
    {
      if (mMode != 0) return false;

      mTmp = mDoc.NewElement(aElement.c_str());
      if (mTmp == nullptr) return false;
      mTmp->SetText(aValue);
      mRoot->InsertEndChild(mTmp);
      mTmp = nullptr;
      return true;
    }
    //////////////////////////////////////////////////////////////////////////////
    bool C_SerializerXML::setString(std::string aElement, std::string aValue)
    {
      if (mMode != 0) return false;

      mTmp = mDoc.NewElement(aElement.c_str());
      if (mTmp == nullptr) return false;
      mTmp->SetText(aValue.c_str());
      mRoot->InsertEndChild(mTmp);
      mTmp = nullptr;
      return true;
    }
    //////////////////////////////////////////////////////////////////////////////
    bool C_SerializerXML::setVector2(std::string aElement, C_Vector2 aValue, C_AttribVector2XML aAttribs)
    {
      if (mMode != 0) return false;

      mTmp = mDoc.NewElement(aElement.c_str());
      if (mTmp == nullptr) return false;
      mTmp->SetAttribute(aAttribs.a.c_str(), aValue.x);
  		mTmp->SetAttribute(aAttribs.b.c_str(), aValue.y);
      mRoot->InsertEndChild(mTmp);
      mTmp = nullptr;
      return true;
    }
    //////////////////////////////////////////////////////////////////////////////
    bool C_SerializerXML::setVector3(std::string aElement, C_Vector3 aValue, C_AttribVector3XML aAttribs)
    {
      if (mMode != 0) return false;

      mTmp = mDoc.NewElement(aElement.c_str());
      if (mTmp == nullptr) return false;
      mTmp->SetAttribute(aAttribs.a.c_str(), aValue.x);
  		mTmp->SetAttribute(aAttribs.b.c_str(), aValue.y);
      mTmp->SetAttribute(aAttribs.c.c_str(), aValue.z);
      mRoot->InsertEndChild(mTmp);
      mTmp = nullptr;
      return true;
    }
    //////////////////////////////////////////////////////////////////////////////
    bool C_SerializerXML::setVector4(std::string aElement, C_Vector4 aValue, C_AttribVector4XML aAttribs)
    {
      if (mMode != 0) return false;

      mTmp = mDoc.NewElement(aElement.c_str());
      if (mTmp == nullptr) return false;
      mTmp->SetAttribute(aAttribs.a.c_str(), aValue.x);
  		mTmp->SetAttribute(aAttribs.b.c_str(), aValue.y);
      mTmp->SetAttribute(aAttribs.c.c_str(), aValue.z);
      mTmp->SetAttribute(aAttribs.d.c_str(), aValue.w);
      mRoot->InsertEndChild(mTmp);
      mTmp = nullptr;
      return true;
    }
    //////////////////////////////////////////////////////////////////////////////
    bool C_SerializerXML::save()
    {
      if (mMode != 0) return false;
      if (mDoc.SaveFile(mFile.c_str()) != C_XML_SUCCESS) return false;
      return true;
    }
    //////////////////////////////////////////////////////////////////////////////
    bool C_SerializerXML::load(std::string aFile, std::string aRoot)
    {
      mMode = 1;
      if (!aFile.empty()) mFile = aFile;
      if (!aRoot.empty()) mRootName = aRoot;
      if (mDoc.LoadFile(mFile.c_str()) != C_XML_SUCCESS) return false;
      mRoot = mDoc.FirstChildElement(mRootName.c_str());
      if (mRoot == nullptr) return false;
      return true;
    }
    //////////////////////////////////////////////////////////////////////////////
    bool C_SerializerXML::getInt(std::string aElement, int* aValue)
    {
      if (mRoot == nullptr) return false;
      mTmp = mRoot->FirstChildElement(aElement.c_str());
      if (mTmp == nullptr) return false;
      mTmp->QueryIntText(aValue);
      return true;
    }
    //////////////////////////////////////////////////////////////////////////////
    bool C_SerializerXML::getBool(std::string aElement, bool* aValue)
    {
      if (mRoot == nullptr) return false;
      mTmp = mRoot->FirstChildElement(aElement.c_str());
      if (mTmp == nullptr) return false;
      mTmp->QueryBoolText(aValue);
      return true;
    }
    //////////////////////////////////////////////////////////////////////////////
    bool C_SerializerXML::getFloat(std::string aElement, float* aValue)
    {
      if (mRoot == nullptr) return false;
      mTmp = mRoot->FirstChildElement(aElement.c_str());
      if (mTmp == nullptr) return false;
      mTmp->QueryFloatText(aValue);
      return true;
    }
    //////////////////////////////////////////////////////////////////////////////
    bool C_SerializerXML::getDouble(std::string aElement, double* aValue)
    {
      if (mRoot == nullptr) return false;
      mTmp = mRoot->FirstChildElement(aElement.c_str());
      if (mTmp == nullptr) return false;
      mTmp->QueryDoubleText(aValue);
      return true;
    }
    //////////////////////////////////////////////////////////////////////////////
    bool C_SerializerXML::getString(std::string aElement, std::string* aValue)
    {
      if (aValue == nullptr) return false;
      if (mRoot == nullptr) return false;
      mTmp = mRoot->FirstChildElement(aElement.c_str());
      if (mTmp == nullptr) return false;
      *aValue = mTmp->GetText();
      return true;
    }
    //////////////////////////////////////////////////////////////////////////////
    bool C_SerializerXML::getVector2(std::string aElement, C_Vector2* aValue, C_AttribVector2XML aAttrib)
    {
      if (mRoot == nullptr) return false;
      mTmp = mRoot->FirstChildElement(aElement.c_str());
      if (mTmp == nullptr) return false;
      mTmp->QueryFloatAttribute(aAttrib.a.c_str(), &aValue->x);
      mTmp->QueryFloatAttribute(aAttrib.b.c_str(), &aValue->y);
      return true;
    }
    //////////////////////////////////////////////////////////////////////////////
    bool C_SerializerXML::getVector3(std::string aElement, C_Vector3* aValue, C_AttribVector3XML aAttrib)
    {
      if (mRoot == nullptr) return false;
      mTmp = mRoot->FirstChildElement(aElement.c_str());
      if (mTmp == nullptr) return false;
      mTmp->QueryFloatAttribute(aAttrib.a.c_str(), &aValue->x);
      mTmp->QueryFloatAttribute(aAttrib.b.c_str(), &aValue->y);
      mTmp->QueryFloatAttribute(aAttrib.c.c_str(), &aValue->z);
      return true;
    }
    //////////////////////////////////////////////////////////////////////////////
    bool C_SerializerXML::getVector4(std::string aElement, C_Vector4* aValue, C_AttribVector4XML aAttrib)
    {
      if (mRoot == nullptr) return false;
      mTmp = mRoot->FirstChildElement(aElement.c_str());
      if (mTmp == nullptr) return false;
      mTmp->QueryFloatAttribute(aAttrib.a.c_str(), &aValue->x);
      mTmp->QueryFloatAttribute(aAttrib.b.c_str(), &aValue->y);
      mTmp->QueryFloatAttribute(aAttrib.c.c_str(), &aValue->z);
      mTmp->QueryFloatAttribute(aAttrib.d.c_str(), &aValue->w);
      return true;
    }
    //////////////////////////////////////////////////////////////////////////////
    C_SerializerXML::~C_SerializerXML()
    {

    }

  }

}
