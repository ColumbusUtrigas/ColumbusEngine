/************************************************
*              	  Serializer.h                  *
*************************************************
*          This file is a part of:              *
*               COLUMBUS ENGINE                 *
*************************************************
*                Nika(Columbus) Red             *
*                   18.10.2017                  *
*************************************************/

#pragma once

#include <System/System.h>
#include <Math/Vector2.h>
#include <Math/Vector3.h>
#include <Math/Vector4.h>

namespace Columbus
{

  namespace Serializer
  {

    class C_SerializerXML
    {
    private:
      C_XMLDoc mDoc;
  		C_XMLNode* mRoot = nullptr;
      C_XMLElement* mTmp = nullptr;

      std::string mFile;
      std::string mRootName;

      struct C_AttribVector2XML { std::string a, b; };
      struct C_AttribVector3XML { std::string a, b, c; };
      struct C_AttribVector4XML { std::string a, b, c, d; };

      bool mInited = false;

      int mMode = 0; // 0 - saving, 1 - loading
    public:
      enum C_XMLMode { C_XML_SERIALIZATION, C_XML_DESERIALIZATION };
    public:
      C_SerializerXML();
      C_SerializerXML(std::string aFile, std::string aRoot, C_XMLMode aMode);

      bool write(std::string aFile, std::string aRoot);
      bool setInt(std::string aElement, int aValue);
      bool setBool(std::string aElement, bool aValue);
      bool setFloat(std::string aElement, float aValue);
      bool setDouble(std::string aElement, double aValue);
      bool setString(std::string aElement, std::string aValue);
      bool setVector2(std::string aElement, C_Vector2 aValue, C_AttribVector2XML aAttribs);
      bool setVector3(std::string aElement, C_Vector3 aValue, C_AttribVector3XML aAttribs);
      bool setVector4(std::string aElement, C_Vector4 aValue, C_AttribVector4XML aAttribs);
      bool save();

      bool read(std::string aFile, std::string aRoot);
      bool getInt(std::string aElement, int* aValue);
      bool getBool(std::string aElement, bool* aValue);
      bool getFloat(std::string aElement, float* aValue);
      bool getDouble(std::string aElement, double* aValue);
      bool getString(std::string aElement, std::string* aValue);
      bool getVector2(std::string aElement, C_Vector2* aValue, C_AttribVector2XML aAttribs);
      bool getVector3(std::string aElement, C_Vector3* aValue, C_AttribVector3XML aAttribs);
      bool getVector4(std::string aElement, C_Vector4* aValue, C_AttribVector4XML aAttribs);

      ~C_SerializerXML();
    };

  }

}
