/************************************************
*              	 Serializer.cpp                 *
*************************************************
*          This file is a part of:              *
*               COLUMBUS ENGINE                 *
*************************************************
*                Nika(Columbus) Red             *
*                   18.10.2017                  *
*************************************************/

#include <System/Serializer.h>

namespace Columbus
{

	namespace Serializer
	{


		//////////////////////////////////////////////////////////////////////////////
		C_XMLElement* SerializerXML::getElementFromHierarchy(std::vector<std::string> aElement)
		{
			std::string end = aElement[aElement.size() - 1];

			mTmp = mRoot->FirstChildElement(aElement[0].c_str());

			for (auto Name : aElement)
				if (Name != aElement[0] && Name != end)
					if (mTmp != nullptr)
						mTmp = mTmp->FirstChildElement(Name.c_str());
					else return nullptr;
			return mTmp;
		}
		//////////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////////
		SerializerXML::SerializerXML()
		{

		}
		//////////////////////////////////////////////////////////////////////////////
		SerializerXML::SerializerXML(std::string aFile, std::string aRoot, C_XMLMode aMode)
		{
			switch (aMode)
			{
			case C_XML_SERIALIZATION:     write(aFile, aRoot);   break;
			case C_XML_DESERIALIZATION:   read(aFile, aRoot);    break;
			}
		}
		//////////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////////
		SerializerXML::Element* SerializerXML::getElement(std::string aElement)
		{
			if (mMode != 1) return nullptr;
			if (mInited == false) return nullptr;
			if (mRoot == nullptr) return nullptr;

			SerializerXML::Element* elem = new SerializerXML::Element;
			elem->elem = mRoot->FirstChildElement(aElement.c_str());

			return elem;
		}
		//////////////////////////////////////////////////////////////////////////////
		SerializerXML::Element* SerializerXML::getElement(Element* aElement, std::string aName)
		{
			if (mMode != 1) return nullptr;
			if (mInited == false) return nullptr;
			if (mRoot == nullptr) return nullptr;
			if (aElement == nullptr) return nullptr;

			SerializerXML::Element* elem = new SerializerXML::Element;
			elem->elem = aElement->elem->FirstChildElement(aName.c_str());

			return elem;
		}
		//////////////////////////////////////////////////////////////////////////////
		bool SerializerXML::getInt(const SerializerXML::Element* aElement, int* aValue)
		{
			if (mMode != 1) return false;
			if (mInited == false) return false;
			if (mRoot == nullptr) return false;
			if (aValue == nullptr) return false;

			if (aElement->elem == nullptr) return false;
			aElement->elem->QueryIntText(aValue);
			return true;
		}
		//////////////////////////////////////////////////////////////////////////////
		bool SerializerXML::getBool(const Element* aElement, bool* aValue)
		{
			if (mMode != 1) return false;
			if (mInited == false) return false;
			if (mRoot == nullptr) return false;
			if (aValue == nullptr) return false;

			if (aElement->elem == nullptr) return false;
			aElement->elem->QueryBoolText(aValue);
			return true;
		}
		//////////////////////////////////////////////////////////////////////////////
		bool SerializerXML::getFloat(const Element* aElement, float* aValue)
		{
			if (mMode != 1) return false;
			if (mInited == false) return false;
			if (mRoot == nullptr) return false;
			if (aValue == nullptr) return false;

			if (aElement->elem == nullptr) return false;
			aElement->elem->QueryFloatText(aValue);
			return true;
		}
		//////////////////////////////////////////////////////////////////////////////
		bool SerializerXML::getDouble(const Element* aElement, double* aValue)
		{
			if (mMode != 1) return false;
			if (mInited == false) return false;
			if (mRoot == nullptr) return false;
			if (aValue == nullptr) return false;

			if (aElement->elem == nullptr) return false;
			aElement->elem->QueryDoubleText(aValue);
			return true;
		}
		//////////////////////////////////////////////////////////////////////////////
		bool SerializerXML::getString(const Element* aElement, std::string* aValue)
		{
			if (mMode != 1) return false;
			if (mInited == false) return false;
			if (mRoot == nullptr) return false;
			if (aValue == nullptr) return false;

			if (aElement->elem == nullptr) return false;
			*aValue = aElement->elem->GetText();
			return true;
		}
		//////////////////////////////////////////////////////////////////////////////
		bool SerializerXML::getVector2(const Element* aElement, Vector2* aValue, C_AttribVector2XML aAttribs)
		{
			if (mMode != 1) return false;
			if (mInited == false) return false;
			if (mRoot == nullptr) return false;
			if (aValue == nullptr) return false;

			if (aElement->elem == nullptr) return false;
			aElement->elem->QueryFloatAttribute(aAttribs.a.c_str(), &aValue->x);
			aElement->elem->QueryFloatAttribute(aAttribs.b.c_str(), &aValue->y);
			return true;
		}
		//////////////////////////////////////////////////////////////////////////////
		bool SerializerXML::getVector3(const Element* aElement, Vector3* aValue, C_AttribVector3XML aAttribs)
		{
			if (mMode != 1) return false;
			if (mInited == false) return false;
			if (mRoot == nullptr) return false;
			if (aValue == nullptr) return false;

			if (aElement->elem == nullptr) return false;
			aElement->elem->QueryFloatAttribute(aAttribs.a.c_str(), &aValue->x);
			aElement->elem->QueryFloatAttribute(aAttribs.b.c_str(), &aValue->y);
			aElement->elem->QueryFloatAttribute(aAttribs.c.c_str(), &aValue->z);
			return true;
		}
		//////////////////////////////////////////////////////////////////////////////
		bool SerializerXML::getVector4(const Element* aElement, Vector4* aValue, C_AttribVector4XML aAttribs)
		{
			if (mMode != 1) return false;
			if (mInited == false) return false;
			if (mRoot == nullptr) return false;
			if (aValue == nullptr) return false;

			if (aElement->elem == nullptr) return false;
			aElement->elem->QueryFloatAttribute(aAttribs.a.c_str(), &aValue->x);
			aElement->elem->QueryFloatAttribute(aAttribs.b.c_str(), &aValue->y);
			aElement->elem->QueryFloatAttribute(aAttribs.c.c_str(), &aValue->z);
			aElement->elem->QueryFloatAttribute(aAttribs.d.c_str(), &aValue->w);
			return true;
		}
		//////////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////////
		bool SerializerXML::write(std::string aFile, std::string aRoot)
		{
			mMode = 0;
			mInited = false;
			mFile = aFile;
			mRootName = aRoot;
			mRoot = mDoc.NewElement(aRoot.c_str());
			if (mRoot == nullptr) return false;
			mDoc.InsertFirstChild(mRoot);
			mInited = true;
			return true;
		}
		//////////////////////////////////////////////////////////////////////////////
		bool SerializerXML::setEmpty(std::string aElement)
		{
			if (mMode != 0) return false;
			if (mInited == false) return false;
			if (mRoot == nullptr) return false;

			mTmp = mDoc.NewElement(aElement.c_str());
			if (mTmp == nullptr) return false;
			mRoot->InsertEndChild(mTmp);
			mTmp = nullptr;
			return true;
		}
		//////////////////////////////////////////////////////////////////////////////
		bool SerializerXML::setSubEmpty(std::vector<std::string> aElement)
		{
			if (mMode != 0) return false;
			if (mInited == false) return false;
			if (mRoot == nullptr) return false;

			std::string end = aElement[aElement.size() - 1];
			C_XMLElement* subElement = mDoc.NewElement(end.c_str());

			if (getElementFromHierarchy(aElement) == nullptr) return false;
			mTmp->InsertEndChild(subElement);
			return true;
		}
		//////////////////////////////////////////////////////////////////////////////
		bool SerializerXML::setInt(std::string aElement, int aValue)
		{
			if (mMode != 0) return false;
			if (mInited == false) return false;
			if (mRoot == nullptr) return false;

			mTmp = mDoc.NewElement(aElement.c_str());
			if (mTmp == nullptr) return false;
			mTmp->SetText(aValue);
			mRoot->InsertEndChild(mTmp);
			mTmp = nullptr;
			return true;
		}
		//////////////////////////////////////////////////////////////////////////////
		bool SerializerXML::setSubInt(std::vector<std::string> aElement, int aValue)
		{
			if (mMode != 0) return false;
			if (mInited == false) return false;
			if (mRoot == nullptr) return false;

			std::string end = aElement[aElement.size() - 1];
			C_XMLElement* subElement = mDoc.NewElement(end.c_str());

			if (getElementFromHierarchy(aElement) == nullptr) return false;
			subElement->SetText(aValue);
			mTmp->InsertEndChild(subElement);
			return true;
		}
		//////////////////////////////////////////////////////////////////////////////
		bool SerializerXML::setBool(std::string aElement, bool aValue)
		{
			if (mMode != 0) return false;
			if (mInited == false) return false;
			if (mRoot == nullptr) return false;

			mTmp = mDoc.NewElement(aElement.c_str());
			if (mTmp == nullptr) return false;
			mTmp->SetText(aValue);
			mRoot->InsertEndChild(mTmp);
			mTmp = nullptr;
			return true;
		}
		//////////////////////////////////////////////////////////////////////////////
		bool SerializerXML::setSubBool(std::vector<std::string> aElement, bool aValue)
		{
			if (mMode != 0) return false;
			if (mInited == false) return false;
			if (mRoot == nullptr) return false;

			std::string end = aElement[aElement.size() - 1];
			C_XMLElement* subElement = mDoc.NewElement(end.c_str());

			if (getElementFromHierarchy(aElement) == nullptr) return false;
			subElement->SetText(aValue);
			mTmp->InsertEndChild(subElement);
			return true;
		}
		//////////////////////////////////////////////////////////////////////////////
		bool SerializerXML::setFloat(std::string aElement, float aValue)
		{
			if (mMode != 0) return false;
			if (mInited == false) return false;
			if (mRoot == nullptr) return false;

			mTmp = mDoc.NewElement(aElement.c_str());
			if (mTmp == nullptr) return false;
			mTmp->SetText(aValue);
			mRoot->InsertEndChild(mTmp);
			mTmp = nullptr;
			return true;
		}
		//////////////////////////////////////////////////////////////////////////////
		bool SerializerXML::setSubFloat(std::vector<std::string> aElement, float aValue)
		{
			if (mMode != 0) return false;
			if (mInited == false) return false;
			if (mRoot == nullptr) return false;

			std::string end = aElement[aElement.size() - 1];
			C_XMLElement* subElement = mDoc.NewElement(end.c_str());

			if (getElementFromHierarchy(aElement) == nullptr) return false;
			subElement->SetText(aValue);
			mTmp->InsertEndChild(subElement);
			return true;
		}
		//////////////////////////////////////////////////////////////////////////////
		bool SerializerXML::setDouble(std::string aElement, double aValue)
		{
			if (mMode != 0) return false;
			if (mInited == false) return false;
			if (mRoot == nullptr) return false;

			mTmp = mDoc.NewElement(aElement.c_str());
			if (mTmp == nullptr) return false;
			mTmp->SetText(aValue);
			mRoot->InsertEndChild(mTmp);
			mTmp = nullptr;
			return true;
		}
		//////////////////////////////////////////////////////////////////////////////
		bool SerializerXML::setSubDouble(std::vector<std::string> aElement, double aValue)
		{
			if (mMode != 0) return false;
			if (mInited == false) return false;
			if (mRoot == nullptr) return false;

			std::string end = aElement[aElement.size() - 1];
			C_XMLElement* subElement = mDoc.NewElement(end.c_str());

			if (getElementFromHierarchy(aElement) == nullptr) return false;
			subElement->SetText(aValue);
			mTmp->InsertEndChild(subElement);
			return true;
		}
		//////////////////////////////////////////////////////////////////////////////
		bool SerializerXML::setString(std::string aElement, std::string aValue)
		{
			if (mMode != 0) return false;
			if (mInited == false) return false;
			if (mRoot == nullptr) return false;

			mTmp = mDoc.NewElement(aElement.c_str());
			if (mTmp == nullptr) return false;
			mTmp->SetText(aValue.c_str());
			mRoot->InsertEndChild(mTmp);
			mTmp = nullptr;
			return true;
		}
		//////////////////////////////////////////////////////////////////////////////
		bool SerializerXML::setSubString(std::vector<std::string> aElement, std::string aValue)
		{
			if (mMode != 0) return false;
			if (mInited == false) return false;
			if (mRoot == nullptr) return false;

			std::string end = aElement[aElement.size() - 1];
			C_XMLElement* subElement = mDoc.NewElement(end.c_str());

			if (getElementFromHierarchy(aElement) == nullptr) return false;
			subElement->SetText(aValue.c_str());
			mTmp->InsertEndChild(subElement);
			return true;
		}
		//////////////////////////////////////////////////////////////////////////////
		bool SerializerXML::setVector2(std::string aElement, Vector2 aValue, C_AttribVector2XML aAttribs)
		{
			if (mMode != 0) return false;
			if (mInited == false) return false;
			if (mRoot == nullptr) return false;

			mTmp = mDoc.NewElement(aElement.c_str());
			if (mTmp == nullptr) return false;
			mTmp->SetAttribute(aAttribs.a.c_str(), aValue.x);
			mTmp->SetAttribute(aAttribs.b.c_str(), aValue.y);
			mRoot->InsertEndChild(mTmp);
			mTmp = nullptr;
			return true;
		}
		//////////////////////////////////////////////////////////////////////////////
		bool SerializerXML::setSubVector2(std::vector<std::string> aElement, Vector2 aValue, C_AttribVector2XML aAttribs)
		{
			if (mMode != 0) return false;
			if (mInited == false) return false;
			if (mRoot == nullptr) return false;

			std::string end = aElement[aElement.size() - 1];
			C_XMLElement* subElement = mDoc.NewElement(end.c_str());

			if (getElementFromHierarchy(aElement) == nullptr) return false;
			subElement->SetAttribute(aAttribs.a.c_str(), aValue.x);
			subElement->SetAttribute(aAttribs.b.c_str(), aValue.y);
			mTmp->InsertEndChild(subElement);
			return true;
		}
		//////////////////////////////////////////////////////////////////////////////
		bool SerializerXML::setVector3(std::string aElement, Vector3 aValue, C_AttribVector3XML aAttribs)
		{
			if (mMode != 0) return false;
			if (mInited == false) return false;
			if (mRoot == nullptr) return false;

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
		bool SerializerXML::setSubVector3(std::vector<std::string> aElement, Vector3 aValue, C_AttribVector3XML aAttribs)
		{
			if (mMode != 0) return false;
			if (mInited == false) return false;
			if (mRoot == nullptr) return false;

			std::string end = aElement[aElement.size() - 1];
			C_XMLElement* subElement = mDoc.NewElement(end.c_str());

			if (getElementFromHierarchy(aElement) == nullptr) return false;
			subElement->SetAttribute(aAttribs.a.c_str(), aValue.x);
			subElement->SetAttribute(aAttribs.b.c_str(), aValue.y);
			subElement->SetAttribute(aAttribs.c.c_str(), aValue.z);
			mTmp->InsertEndChild(subElement);
			return true;
		}
		//////////////////////////////////////////////////////////////////////////////
		bool SerializerXML::setVector4(std::string aElement, Vector4 aValue, C_AttribVector4XML aAttribs)
		{
			if (mMode != 0) return false;
			if (mInited == false) return false;
			if (mRoot == nullptr) return false;

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
		bool SerializerXML::setSubVector4(std::vector<std::string> aElement, Vector4 aValue, C_AttribVector4XML aAttribs)
		{
			if (mMode != 0) return false;
			if (mInited == false) return false;
			if (mRoot == nullptr) return false;

			std::string end = aElement[aElement.size() - 1];
			C_XMLElement* subElement = mDoc.NewElement(end.c_str());

			if (getElementFromHierarchy(aElement) == nullptr) return false;
			subElement->SetAttribute(aAttribs.a.c_str(), aValue.x);
			subElement->SetAttribute(aAttribs.b.c_str(), aValue.y);
			subElement->SetAttribute(aAttribs.c.c_str(), aValue.z);
			subElement->SetAttribute(aAttribs.d.c_str(), aValue.w);
			mTmp->InsertEndChild(subElement);
			return true;
		}
		//////////////////////////////////////////////////////////////////////////////
		bool SerializerXML::save()
		{
			if (mMode != 0) return false;
			if (mInited == false) return false;
			if (mDoc.SaveFile(mFile.c_str()) != C_XML_SUCCESS) return false;
			return true;
		}
		//////////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////////
		bool SerializerXML::read(std::string aFile, std::string aRoot)
		{
			mMode = 1;
			mInited = false;
			if (!aFile.empty()) mFile = aFile;
			if (!aRoot.empty()) mRootName = aRoot;
			if (mDoc.LoadFile(mFile.c_str()) != C_XML_SUCCESS) return false;
			mRoot = mDoc.FirstChildElement(mRootName.c_str());
			if (mRoot == nullptr) return false;
			mInited = true;
			return true;
		}
		//////////////////////////////////////////////////////////////////////////////
		bool SerializerXML::getInt(std::string aElement, int* aValue)
		{
			if (mMode != 1) return false;
			if (mInited == false) return false;
			if (mRoot == nullptr) return false;
			if (aValue == nullptr) return false;

			mTmp = mRoot->FirstChildElement(aElement.c_str());
			if (mTmp == nullptr) return false;
			mTmp->QueryIntText(aValue);
			return true;
		}
		//////////////////////////////////////////////////////////////////////////////
		bool SerializerXML::getSubInt(std::vector<std::string> aElement, int* aValue)
		{
			if (mMode != 1) return false;
			if (mInited == false) return false;
			if (mRoot == nullptr) return false;
			if (aValue == nullptr) return false;

			if (getElementFromHierarchy(aElement) == nullptr) return false;
			mTmp = mTmp->FirstChildElement(aElement[aElement.size() - 1].c_str());
			mTmp->QueryIntText(aValue);
			return true;
		}
		//////////////////////////////////////////////////////////////////////////////
		bool SerializerXML::getBool(std::string aElement, bool* aValue)
		{
			if (mMode != 1) return false;
			if (mInited == false) return false;
			if (mRoot == nullptr) return false;
			if (aValue == nullptr) return false;

			mTmp = mRoot->FirstChildElement(aElement.c_str());
			if (mTmp == nullptr) return false;
			mTmp->QueryBoolText(aValue);
			return true;
		}
		//////////////////////////////////////////////////////////////////////////////
		bool SerializerXML::getSubBool(std::vector<std::string> aElement, bool* aValue)
		{
			if (mMode != 1) return false;
			if (mInited == false) return false;
			if (mRoot == nullptr) return false;
			if (aValue == nullptr) return false;

			if (getElementFromHierarchy(aElement) == nullptr) return false;
			mTmp = mTmp->FirstChildElement(aElement[aElement.size() - 1].c_str());
			mTmp->QueryBoolText(aValue);
			return true;
		}
		//////////////////////////////////////////////////////////////////////////////
		bool SerializerXML::getFloat(std::string aElement, float* aValue)
		{
			if (mMode != 1) return false;
			if (mInited == false) return false;
			if (mRoot == nullptr) return false;
			if (aValue == nullptr) return false;

			mTmp = mRoot->FirstChildElement(aElement.c_str());
			if (mTmp == nullptr) return false;
			mTmp->QueryFloatText(aValue);
			return true;
		}
		//////////////////////////////////////////////////////////////////////////////
		bool SerializerXML::getSubFloat(std::vector<std::string> aElement, float* aValue)
		{
			if (mMode != 1) return false;
			if (mInited == false) return false;
			if (mRoot == nullptr) return false;
			if (aValue == nullptr) return false;

			if (getElementFromHierarchy(aElement) == nullptr) return false;
			mTmp = mTmp->FirstChildElement(aElement[aElement.size() - 1].c_str());
			mTmp->QueryFloatText(aValue);
			return true;
		}
		//////////////////////////////////////////////////////////////////////////////
		bool SerializerXML::getDouble(std::string aElement, double* aValue)
		{
			if (mMode != 1) return false;
			if (mInited == false) return false;
			if (mRoot == nullptr) return false;
			if (aValue == nullptr) return false;

			mTmp = mRoot->FirstChildElement(aElement.c_str());
			if (mTmp == nullptr) return false;
			mTmp->QueryDoubleText(aValue);
			return true;
		}
		//////////////////////////////////////////////////////////////////////////////
		bool SerializerXML::getSubDouble(std::vector<std::string> aElement, double* aValue)
		{
			if (mMode != 1) return false;
			if (mInited == false) return false;
			if (mRoot == nullptr) return false;
			if (aValue == nullptr) return false;

			if (getElementFromHierarchy(aElement) == nullptr) return false;
			mTmp = mTmp->FirstChildElement(aElement[aElement.size() - 1].c_str());
			mTmp->QueryDoubleText(aValue);
			return true;
		}
		//////////////////////////////////////////////////////////////////////////////
		bool SerializerXML::getString(std::string aElement, std::string* aValue)
		{
			if (mMode != 1) return false;
			if (mInited == false) return false;
			if (mRoot == nullptr) return false;
			if (aValue == nullptr) return false;

			mTmp = mRoot->FirstChildElement(aElement.c_str());
			if (mTmp == nullptr) return false;
			*aValue = mTmp->GetText();
			return true;
		}
		//////////////////////////////////////////////////////////////////////////////
		bool SerializerXML::getSubString(std::vector<std::string> aElement, std::string* aValue)
		{
			if (mMode != 1) return false;
			if (mInited == false) return false;
			if (mRoot == nullptr) return false;
			if (aValue == nullptr) return false;

			if (getElementFromHierarchy(aElement) == nullptr) return false;
			mTmp = mTmp->FirstChildElement(aElement[aElement.size() - 1].c_str());
			*aValue = mTmp->GetText();
			return true;
		}
		//////////////////////////////////////////////////////////////////////////////
		bool SerializerXML::getVector2(std::string aElement, Vector2* aValue, C_AttribVector2XML aAttrib)
		{
			if (mMode != 1) return false;
			if (mInited == false) return false;
			if (mRoot == nullptr) return false;
			if (aValue == nullptr) return false;

			mTmp = mRoot->FirstChildElement(aElement.c_str());
			if (mTmp == nullptr) return false;
			mTmp->QueryFloatAttribute(aAttrib.a.c_str(), &aValue->x);
			mTmp->QueryFloatAttribute(aAttrib.b.c_str(), &aValue->y);
			return true;
		}
		//////////////////////////////////////////////////////////////////////////////
		bool SerializerXML::getSubVector2(std::vector<std::string> aElement, Vector2* aValue, C_AttribVector2XML aAttrib)
		{
			if (mMode != 1) return false;
			if (mInited == false) return false;
			if (mRoot == nullptr) return false;
			if (aValue == nullptr) return false;

			if (getElementFromHierarchy(aElement) == nullptr) return false;
			mTmp = mTmp->FirstChildElement(aElement[aElement.size() - 1].c_str());
			mTmp->QueryFloatAttribute(aAttrib.a.c_str(), &aValue->x);
			mTmp->QueryFloatAttribute(aAttrib.b.c_str(), &aValue->y);
			return true;
		}
		//////////////////////////////////////////////////////////////////////////////
		bool SerializerXML::getVector3(std::string aElement, Vector3* aValue, C_AttribVector3XML aAttrib)
		{
			if (mMode != 1) return false;
			if (mInited == false) return false;
			if (mRoot == nullptr) return false;
			if (aValue == nullptr) return false;

			mTmp = mRoot->FirstChildElement(aElement.c_str());
			if (mTmp == nullptr) return false;
			mTmp->QueryFloatAttribute(aAttrib.a.c_str(), &aValue->x);
			mTmp->QueryFloatAttribute(aAttrib.b.c_str(), &aValue->y);
			mTmp->QueryFloatAttribute(aAttrib.c.c_str(), &aValue->z);
			return true;
		}
		//////////////////////////////////////////////////////////////////////////////
		bool SerializerXML::getSubVector3(std::vector<std::string> aElement, Vector3* aValue, C_AttribVector3XML aAttrib)
		{
			if (mMode != 1) return false;
			if (mInited == false) return false;
			if (mRoot == nullptr) return false;
			if (aValue == nullptr) return false;

			if (getElementFromHierarchy(aElement) == nullptr) return false;
			mTmp = mTmp->FirstChildElement(aElement[aElement.size() - 1].c_str());
			mTmp->QueryFloatAttribute(aAttrib.a.c_str(), &aValue->x);
			mTmp->QueryFloatAttribute(aAttrib.b.c_str(), &aValue->y);
			mTmp->QueryFloatAttribute(aAttrib.c.c_str(), &aValue->z);
			return true;
		}
		//////////////////////////////////////////////////////////////////////////////
		bool SerializerXML::getVector4(std::string aElement, Vector4* aValue, C_AttribVector4XML aAttrib)
		{
			if (mMode != 1) return false;
			if (mInited == false) return false;
			if (mRoot == nullptr) return false;
			if (aValue == nullptr) return false;

			mTmp = mRoot->FirstChildElement(aElement.c_str());
			if (mTmp == nullptr) return false;
			mTmp->QueryFloatAttribute(aAttrib.a.c_str(), &aValue->x);
			mTmp->QueryFloatAttribute(aAttrib.b.c_str(), &aValue->y);
			mTmp->QueryFloatAttribute(aAttrib.c.c_str(), &aValue->z);
			mTmp->QueryFloatAttribute(aAttrib.d.c_str(), &aValue->w);
			return true;
		}
		//////////////////////////////////////////////////////////////////////////////
		bool SerializerXML::getSubVector4(std::vector<std::string> aElement, Vector4* aValue, C_AttribVector4XML aAttrib)
		{
			if (mMode != 1) return false;
			if (mInited == false) return false;
			if (mRoot == nullptr) return false;
			if (aValue == nullptr) return false;

			if (getElementFromHierarchy(aElement) == nullptr) return false;
			mTmp = mTmp->FirstChildElement(aElement[aElement.size() - 1].c_str());
			mTmp->QueryFloatAttribute(aAttrib.a.c_str(), &aValue->x);
			mTmp->QueryFloatAttribute(aAttrib.b.c_str(), &aValue->y);
			mTmp->QueryFloatAttribute(aAttrib.c.c_str(), &aValue->z);
			mTmp->QueryFloatAttribute(aAttrib.d.c_str(), &aValue->w);
			return true;
		}
		//////////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////////
		SerializerXML::~SerializerXML()
		{

		}

	}

}
