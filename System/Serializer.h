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

			struct AttribVector2XML { std::string a, b; };
			struct AttribVector3XML { std::string a, b, c; };
			struct AttribVector4XML { std::string a, b, c, d; };

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

			bool GetInt(const Element* InElement, int& OutValue);
			bool GetBool(const Element* InElement, bool& OutValue);
			bool GetFloat(const Element* InElement, float& OutValue);
			bool GetDouble(const Element* InElement, double& OutValue);
			bool GetString(const Element* InElement, std::string& OutValue);
			bool GetVector2(const Element* InElement, Vector2& OutValue, AttribVector2XML Attribs);
			bool GetVector3(const Element* InElement, Vector3& OutValue, AttribVector3XML Attribs);
			bool GetVector4(const Element* InElement, Vector4& OutValue, AttribVector4XML Attribs);

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
			bool SetVector2(std::string aElement, Vector2 aValue, AttribVector2XML aAttribs);
			bool SetSubVector2(std::vector<std::string> aElement, Vector2 aValue, AttribVector2XML aAttribs);
			bool SetVector3(std::string aElement, Vector3 aValue, AttribVector3XML aAttribs);
			bool SetSubVector3(std::vector<std::string> aElement, Vector3 aValue, AttribVector3XML aAttribs);
			bool SetVector4(std::string aElement, Vector4 aValue, AttribVector4XML aAttribs);
			bool SetSubVector4(std::vector<std::string> aElement, Vector4 aValue, AttribVector4XML aAttribs);
			bool Save();

			bool Read(std::string aFile, std::string aRoot);
			bool GetInt(std::string InElement, int& OutValue);
			bool GetSubInt(std::vector<std::string> InElement, int& OutValue);
			bool GetBool(std::string InElement, bool& OutValue);
			bool GetSubBool(std::vector<std::string> InElement, bool& OutValue);
			bool GetFloat(std::string InElement, float& OutValue);
			bool GetSubFloat(std::vector<std::string> InElement, float& OutValue);
			bool GetDouble(std::string InElement, double& OutValue);
			bool GetSubDouble(std::vector<std::string> InElement, double& OutValue);
			bool GetString(std::string InElement, std::string& OutValue);
			bool GetSubString(std::vector<std::string> InElement, std::string& OutValue);
			bool GetVector2(std::string InElement, Vector2& OutValue, AttribVector2XML Attribs);
			bool GetSubVector2(std::vector<std::string> InElement, Vector2& OutValue, AttribVector2XML Attribs);
			bool GetVector3(std::string InElement, Vector3& OutValue, AttribVector3XML Attribs);
			bool GetSubVector3(std::vector<std::string> InElement, Vector3& OutValue, AttribVector3XML Attribs);
			bool GetVector4(std::string InElement, Vector4& OutValue, AttribVector4XML Attribs);
			bool GetSubVector4(std::vector<std::string> InElement, Vector4& OutValue, AttribVector4XML Attribs);

			~SerializerXML();
		};

		#define XML_NEXT_ELEMENT(a, b) \
			a->elem = a->elem->NextSiblingElement(b); \
			if (a->elem == nullptr) a = nullptr;

	}

}
