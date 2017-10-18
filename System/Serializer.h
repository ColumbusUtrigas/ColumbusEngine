/************************************************
*              	  Serializer.h                  *
*************************************************
*          This file is a part of:              *
*               COLUMBUS ENGINE                 *
*************************************************
*             Nikolay(Columbus) Red             *
*                   18.10.2017                  *
*************************************************/

#pragma once

#include <System/System.h>
#include <Math/Vector2.h>
#include <Math/Vector3.h>
#include <Math/Vector4.h>

namespace C
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

      struct C_AttribVector2XML { std::string a, b; };
      struct C_AttribVector3XML { std::string a, b, c; };
      struct C_AttribVector4XML { std::string a, b, c, d; };
    public:
      C_SerializerXML(std::string aFile, std::string aRoot);

      bool setInt(std::string aElement, int aValue);
      bool setBool(std::string aElement, bool aValue);
      bool setFloat(std::string aElement, float aValue);
      bool setDouble(std::string aElement, double aValue);
      bool setString(std::string aElement, std::string aValue);
      bool setVector2(std::string aElement, C_Vector2 aValue, C_AttribVector2XML aAttribs);
      bool setVector3(std::string aElement, C_Vector3 aValue, C_AttribVector3XML aAttribs);
      bool setVector4(std::string aElement, C_Vector4 aValue, C_AttribVector4XML aAttribs);
      bool save();

      ~C_SerializerXML();
    };

    class C_DeserializerXML
    {
    private:
      C_XMLDoc mDoc;
      C_XMLNode* mRoot = nullptr;
      C_XMLElement* mTmp = nullptr;

      std::string mFile;

      bool mInited = false;

      struct C_AttribVector2XML { std::string a, b; };
      struct C_AttribVector3XML { std::string a, b, c; };
      struct C_AttribVector4XML { std::string a, b, c, d; };
    public:
      C_DeserializerXML(std::string aFile, std::string aRoot);

      bool getInt(std::string aElement, int* aValue);

      ~C_DeserializerXML();
    };

  }

}
