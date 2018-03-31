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
			if (aElement.size() <= 1) return nullptr;

			mTmp = mRoot->FirstChildElement(aElement[0].c_str());

			for (auto Name : aElement)
			{
				if (Name != aElement[0])
				{
					if (mTmp != nullptr)
					{
						mTmp = mTmp->FirstChildElement(Name.c_str());
					} else return nullptr;
				}
			}

			return mTmp;
		}
		//////////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////////
		SerializerXML::SerializerXML()
		{

		}
		//////////////////////////////////////////////////////////////////////////////
		SerializerXML::SerializerXML(std::string aFile, std::string aRoot, XMLMode aMode)
		{
			switch (aMode)
			{
			case XML_SERIALIZATION:     Write(aFile, aRoot);   break;
			case XML_DESERIALIZATION:   Read(aFile, aRoot);    break;
			}
		}
		//////////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////////
		SerializerXML::Element* SerializerXML::GetElement(std::string aElement)
		{
			if (mMode != 1) return nullptr;
			if (mInited == false) return nullptr;
			if (mRoot == nullptr) return nullptr;

			SerializerXML::Element* elem = new SerializerXML::Element;
			elem->elem = mRoot->FirstChildElement(aElement.c_str());

			if (elem->elem == nullptr)
			{
				delete elem;
				return nullptr;
			}

			return elem;
		}
		//////////////////////////////////////////////////////////////////////////////
		SerializerXML::Element* SerializerXML::GetElement(Element* aElement, std::string aName)
		{
			if (mMode != 1) return nullptr;
			if (mInited == false) return nullptr;
			if (mRoot == nullptr) return nullptr;
			if (aElement == nullptr) return nullptr;

			SerializerXML::Element* elem = new SerializerXML::Element;
			elem->elem = aElement->elem->FirstChildElement(aName.c_str());

			if (elem->elem == nullptr)
			{
				delete elem;
				return nullptr;
			}

			return elem;
		}

		SerializerXML::Element* SerializerXML::GetSubElement(std::vector<std::string> Elements)
		{
			if (mMode != 1) return nullptr;
			if (mInited == false) return nullptr;
			if (mRoot == nullptr) return nullptr;
			if (Elements.size() <= 1) return nullptr;

			SerializerXML::Element* elem = new SerializerXML::Element;
			elem = this->GetElement(Elements[0]);

			for (auto Name : Elements)
			{
				if (Name != Elements[0])
				{
					if (elem != nullptr)
					{
						elem = this->GetElement(elem, Name);
					}
					else
					{
						delete elem;
						return nullptr;
					}
				}
			}

			return elem;
		}

		SerializerXML::Element* SerializerXML::GetSubElement(std::vector<std::string> Elements, Element* Elem)
		{
			if (mMode != 1) return nullptr;
			if (mInited == false) return nullptr;
			if (mRoot == nullptr) return nullptr;
			if (Elem == nullptr) return nullptr;
			if (Elements.size() <= 1) return nullptr;
			
			SerializerXML::Element* elem = new SerializerXML::Element;
			elem = this->GetElement(Elem, Elements[0]);

			for (auto Name : Elements)
			{
				if (Name != Elements[0])
				{
					if (elem != nullptr)
					{
						elem = this->GetElement(elem, Name);
					}
					else
					{
						delete elem;
						return nullptr;
					}
				}
			}

			return elem;
		}

		SerializerXML::Element* SerializerXML::NextElement(Element* Elem, std::string Name)
		{
			if (mMode != 1) return nullptr;
			if (mInited == false) return nullptr;
			if (mRoot == nullptr) return nullptr;
			if (Elem == nullptr) return nullptr;

			Elem->elem = Elem->elem->NextSiblingElement(Name.c_str());
			if (Elem->elem == nullptr)
			{
				delete Elem;
				Elem = nullptr;
			}

			return Elem;
		}

		
		bool SerializerXML::GetInt(const SerializerXML::Element* aElement, int* aValue)
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
		bool SerializerXML::GetBool(const Element* aElement, bool* aValue)
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
		bool SerializerXML::GetFloat(const Element* aElement, float* aValue)
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
		bool SerializerXML::GetDouble(const Element* aElement, double* aValue)
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
		bool SerializerXML::GetString(const Element* aElement, std::string* aValue)
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
		bool SerializerXML::GetVector2(const Element* aElement, Vector2* aValue, C_AttribVector2XML aAttribs)
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
		bool SerializerXML::GetVector3(const Element* aElement, Vector3* aValue, C_AttribVector3XML aAttribs)
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
		bool SerializerXML::GetVector4(const Element* aElement, Vector4* aValue, C_AttribVector4XML aAttribs)
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
		bool SerializerXML::Write(std::string aFile, std::string aRoot)
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
		bool SerializerXML::SetEmpty(std::string aElement)
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
		bool SerializerXML::SetSubEmpty(std::vector<std::string> aElement)
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
		bool SerializerXML::SetInt(std::string aElement, int aValue)
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
		bool SerializerXML::SetSubInt(std::vector<std::string> aElement, int aValue)
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
		bool SerializerXML::SetBool(std::string aElement, bool aValue)
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
		bool SerializerXML::SetSubBool(std::vector<std::string> aElement, bool aValue)
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
		bool SerializerXML::SetFloat(std::string aElement, float aValue)
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
		bool SerializerXML::SetSubFloat(std::vector<std::string> aElement, float aValue)
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
		bool SerializerXML::SetDouble(std::string aElement, double aValue)
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
		bool SerializerXML::SetSubDouble(std::vector<std::string> aElement, double aValue)
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
		bool SerializerXML::SetString(std::string aElement, std::string aValue)
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
		bool SerializerXML::SetSubString(std::vector<std::string> aElement, std::string aValue)
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
		bool SerializerXML::SetVector2(std::string aElement, Vector2 aValue, C_AttribVector2XML aAttribs)
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
		bool SerializerXML::SetSubVector2(std::vector<std::string> aElement, Vector2 aValue, C_AttribVector2XML aAttribs)
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
		bool SerializerXML::SetVector3(std::string aElement, Vector3 aValue, C_AttribVector3XML aAttribs)
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
		bool SerializerXML::SetSubVector3(std::vector<std::string> aElement, Vector3 aValue, C_AttribVector3XML aAttribs)
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
		bool SerializerXML::SetVector4(std::string aElement, Vector4 aValue, C_AttribVector4XML aAttribs)
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
		bool SerializerXML::SetSubVector4(std::vector<std::string> aElement, Vector4 aValue, C_AttribVector4XML aAttribs)
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
		bool SerializerXML::Save()
		{
			if (mMode != 0) return false;
			if (mInited == false) return false;
			if (mDoc.SaveFile(mFile.c_str()) != C_XML_SUCCESS) return false;
			return true;
		}
		//////////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////////
		bool SerializerXML::Read(std::string aFile, std::string aRoot)
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
		bool SerializerXML::GetInt(std::string aElement, int* aValue)
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
		bool SerializerXML::GetSubInt(std::vector<std::string> aElement, int* aValue)
		{
			if (mMode != 1) return false;
			if (mInited == false) return false;
			if (mRoot == nullptr) return false;
			if (aValue == nullptr) return false;

			if (getElementFromHierarchy(aElement) == nullptr) return false;
			mTmp->QueryIntText(aValue);
			return true;
		}
		//////////////////////////////////////////////////////////////////////////////
		bool SerializerXML::GetBool(std::string aElement, bool* aValue)
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
		bool SerializerXML::GetSubBool(std::vector<std::string> aElement, bool* aValue)
		{
			if (mMode != 1) return false;
			if (mInited == false) return false;
			if (mRoot == nullptr) return false;
			if (aValue == nullptr) return false;

			if (getElementFromHierarchy(aElement) == nullptr) return false;
			mTmp->QueryBoolText(aValue);
			return true;
		}
		//////////////////////////////////////////////////////////////////////////////
		bool SerializerXML::GetFloat(std::string aElement, float* aValue)
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
		bool SerializerXML::GetSubFloat(std::vector<std::string> aElement, float* aValue)
		{
			if (mMode != 1) return false;
			if (mInited == false) return false;
			if (mRoot == nullptr) return false;
			if (aValue == nullptr) return false;

			if (getElementFromHierarchy(aElement) == nullptr) return false;
			mTmp->QueryFloatText(aValue);
			return true;
		}
		//////////////////////////////////////////////////////////////////////////////
		bool SerializerXML::GetDouble(std::string aElement, double* aValue)
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
		bool SerializerXML::GetSubDouble(std::vector<std::string> aElement, double* aValue)
		{
			if (mMode != 1) return false;
			if (mInited == false) return false;
			if (mRoot == nullptr) return false;
			if (aValue == nullptr) return false;

			if (getElementFromHierarchy(aElement) == nullptr) return false;
			mTmp->QueryDoubleText(aValue);
			return true;
		}
		//////////////////////////////////////////////////////////////////////////////
		bool SerializerXML::GetString(std::string aElement, std::string* aValue)
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
		bool SerializerXML::GetSubString(std::vector<std::string> aElement, std::string* aValue)
		{
			if (mMode != 1) return false;
			if (mInited == false) return false;
			if (mRoot == nullptr) return false;
			if (aValue == nullptr) return false;

			if (getElementFromHierarchy(aElement) == nullptr) return false;
			*aValue = mTmp->GetText();
			return true;
		}
		//////////////////////////////////////////////////////////////////////////////
		bool SerializerXML::GetVector2(std::string aElement, Vector2* aValue, C_AttribVector2XML aAttrib)
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
		bool SerializerXML::GetSubVector2(std::vector<std::string> aElement, Vector2* aValue, C_AttribVector2XML aAttrib)
		{
			if (mMode != 1) return false;
			if (mInited == false) return false;
			if (mRoot == nullptr) return false;
			if (aValue == nullptr) return false;

			if (getElementFromHierarchy(aElement) == nullptr) return false;
			mTmp->QueryFloatAttribute(aAttrib.a.c_str(), &aValue->x);
			mTmp->QueryFloatAttribute(aAttrib.b.c_str(), &aValue->y);
			return true;
		}
		//////////////////////////////////////////////////////////////////////////////
		bool SerializerXML::GetVector3(std::string aElement, Vector3* aValue, C_AttribVector3XML aAttrib)
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
		bool SerializerXML::GetSubVector3(std::vector<std::string> aElement, Vector3* aValue, C_AttribVector3XML aAttrib)
		{
			if (mMode != 1) return false;
			if (mInited == false) return false;
			if (mRoot == nullptr) return false;
			if (aValue == nullptr) return false;

			if (getElementFromHierarchy(aElement) == nullptr) return false;
			mTmp->QueryFloatAttribute(aAttrib.a.c_str(), &aValue->x);
			mTmp->QueryFloatAttribute(aAttrib.b.c_str(), &aValue->y);
			mTmp->QueryFloatAttribute(aAttrib.c.c_str(), &aValue->z);
			return true;
		}
		//////////////////////////////////////////////////////////////////////////////
		bool SerializerXML::GetVector4(std::string aElement, Vector4* aValue, C_AttribVector4XML aAttrib)
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
		bool SerializerXML::GetSubVector4(std::vector<std::string> aElement, Vector4* aValue, C_AttribVector4XML aAttrib)
		{
			if (mMode != 1) return false;
			if (mInited == false) return false;
			if (mRoot == nullptr) return false;
			if (aValue == nullptr) return false;

			if (getElementFromHierarchy(aElement) == nullptr) return false;
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
