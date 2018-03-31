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

		class SerializerXML
		{
		private:
			tinyxml2::XMLDocument mDoc;
			tinyxml2::XMLNode* mRoot = nullptr;
			tinyxml2::XMLElement* mTmp = nullptr;

			std::string mFile;
			std::string mRootName;

			struct C_AttribVector2XML { std::string a, b; };
			struct C_AttribVector3XML { std::string a, b, c; };
			struct C_AttribVector4XML { std::string a, b, c, d; };

			bool mInited = false;

			int mMode = 0; // 0 - saving, 1 - loading

			C_XMLElement* getElementFromHierarchy(std::vector<std::string> aElement);
		public:
			enum XMLMode { XML_SERIALIZATION, XML_DESERIALIZATION };
		public:
			struct Element
			{
				C_XMLElement* elem;
			};

			SerializerXML();
			SerializerXML(std::string aFile, std::string aRoot, XMLMode aMode);

			Element* GetElement(std::string aElement);
			Element* GetElement(Element* aElement, std::string aName);
			Element* GetSubElement(std::vector<std::string> Elements);
			Element* GetSubElement(std::vector<std::string> Elements, Element* Elem);
			Element* NextElement(Element* Elem, std::string Name);

			bool GetInt(const Element* aElement, int* aValue);
			bool GetBool(const Element* aElement, bool* aValue);
			bool GetFloat(const Element* aElement, float* aValue);
			bool GetDouble(const Element* aElement, double* aValue);
			bool GetString(const Element* aElement, std::string* aValue);
			bool GetVector2(const Element* aElement, Vector2* aValue, C_AttribVector2XML aAttribs);
			bool GetVector3(const Element* aElement, Vector3* aValue, C_AttribVector3XML aAttribs);
			bool GetVector4(const Element* aElement, Vector4* aValue, C_AttribVector4XML aAttribs);

			bool Write(std::string aFile, std::string aRoot);
			bool SetEmpty(std::string aElement);
			bool SetSubEmpty(std::vector<std::string> aElement);
			bool SetInt(std::string aElement, int aValue);
			bool SetSubInt(std::vector<std::string> aElement, int aValue);
			bool SetBool(std::string aElement, bool aValue);
			bool SetSubBool(std::vector<std::string> aElement, bool aValue);
			bool SetFloat(std::string aElement, float aValue);
			bool SetSubFloat(std::vector<std::string> aElement, float aValue);
			bool SetDouble(std::string aElement, double aValue);
			bool SetSubDouble(std::vector<std::string> aElement, double aValue);
			bool SetString(std::string aElement, std::string aValue);
			bool SetSubString(std::vector<std::string> aElement, std::string aValue);
			bool SetVector2(std::string aElement, Vector2 aValue, C_AttribVector2XML aAttribs);
			bool SetSubVector2(std::vector<std::string> aElement, Vector2 aValue, C_AttribVector2XML aAttribs);
			bool SetVector3(std::string aElement, Vector3 aValue, C_AttribVector3XML aAttribs);
			bool SetSubVector3(std::vector<std::string> aElement, Vector3 aValue, C_AttribVector3XML aAttribs);
			bool SetVector4(std::string aElement, Vector4 aValue, C_AttribVector4XML aAttribs);
			bool SetSubVector4(std::vector<std::string> aElement, Vector4 aValue, C_AttribVector4XML aAttribs);
			bool Save();

			bool Read(std::string aFile, std::string aRoot);
			bool GetInt(std::string aElement, int* aValue);
			bool GetSubInt(std::vector<std::string> aElement, int* aValue);
			bool GetBool(std::string aElement, bool* aValue);
			bool GetSubBool(std::vector<std::string> aElement, bool* aValue);
			bool GetFloat(std::string aElement, float* aValue);
			bool GetSubFloat(std::vector<std::string> aElement, float* aValue);
			bool GetDouble(std::string aElement, double* aValue);
			bool GetSubDouble(std::vector<std::string> aElement, double* aValue);
			bool GetString(std::string aElement, std::string* aValue);
			bool GetSubString(std::vector<std::string> aElement, std::string* aValue);
			bool GetVector2(std::string aElement, Vector2* aValue, C_AttribVector2XML aAttribs);
			bool GetSubVector2(std::vector<std::string> aElement, Vector2* aValue, C_AttribVector2XML aAttribs);
			bool GetVector3(std::string aElement, Vector3* aValue, C_AttribVector3XML aAttribs);
			bool GetSubVector3(std::vector<std::string> aElement, Vector3* aValue, C_AttribVector3XML aAttribs);
			bool GetVector4(std::string aElement, Vector4* aValue, C_AttribVector4XML aAttribs);
			bool GetSubVector4(std::vector<std::string> aElement, Vector4* aValue, C_AttribVector4XML aAttribs);

			~SerializerXML();
		};

		#define XML_NEXT_ELEMENT(a, b) \
			a->elem = a->elem->NextSiblingElement(b); \
			if (a->elem == nullptr) a = nullptr;

	}

}
