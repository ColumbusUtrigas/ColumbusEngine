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
      mRoot = mDoc.NewElement(aRoot.c_str());
      mDoc.InsertFirstChild(mRoot);
    }
    //////////////////////////////////////////////////////////////////////////////
    bool C_SerializerXML::setInt(std::string aElement, int aValue)
    {
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
      if (mDoc.SaveFile(mFile.c_str()) != C_XML_SUCCESS) return false;
      return true;
    }
    //////////////////////////////////////////////////////////////////////////////
    C_SerializerXML::~C_SerializerXML()
    {

    }
    //////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////
    C_DeserializerXML::C_DeserializerXML(std::string aFile, std::string aRoot)
    {
      mFile = aFile;
      if(mDoc.LoadFile(aFile.c_str()) != C_XML_SUCCESS) printf("asd\n");
      mRoot = mDoc.FirstChildElement(aRoot.c_str());
      if (mRoot == nullptr) return;
      mInited = true;
    }
    //////////////////////////////////////////////////////////////////////////////
    bool C_DeserializerXML::getInt(std::string aElement, int* aValue)
    {
      if (!mInited) return false;
      mTmp = mRoot->FirstChildElement(aElement.c_str());
      if (mTmp == nullptr) return false;
      mTmp->QueryIntText(aValue);
      mTmp = nullptr;
      return true;
    }
    //////////////////////////////////////////////////////////////////////////////
    C_DeserializerXML::~C_DeserializerXML()
    {

    }

  }

}
