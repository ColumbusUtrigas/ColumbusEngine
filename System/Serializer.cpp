#include <System/Serializer.h>

namespace Columbus
{

	namespace Serializer
	{

		tinyxml2::XMLElement* SerializerXML::getElementFromHierarchy(std::vector<std::string> aElement)
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

		
		bool SerializerXML::GetInt(const SerializerXML::Element* InElement, int& OutValue)
		{
			if (mMode != 1) return false;
			if (mInited == false) return false;
			if (mRoot == nullptr) return false;
			if (InElement == nullptr) return false;
			if (InElement->elem == nullptr) return false;

			InElement->elem->QueryIntText(&OutValue);
			return true;
		}
		//////////////////////////////////////////////////////////////////////////////
		bool SerializerXML::GetBool(const Element* InElement, bool& OutValue)
		{
			if (mMode != 1) return false;
			if (mInited == false) return false;
			if (mRoot == nullptr) return false;
			if (InElement == nullptr) return false;
			if (InElement->elem == nullptr) return false;

			InElement->elem->QueryBoolText(&OutValue);
			return true;
		}
		//////////////////////////////////////////////////////////////////////////////
		bool SerializerXML::GetFloat(const Element* InElement, float& OutValue)
		{
			if (mMode != 1) return false;
			if (mInited == false) return false;
			if (mRoot == nullptr) return false;
			if (InElement == nullptr) return false;
			if (InElement->elem == nullptr) return false;

			InElement->elem->QueryFloatText(&OutValue);
			return true;
		}
		//////////////////////////////////////////////////////////////////////////////
		bool SerializerXML::GetDouble(const Element* InElement, double& OutValue)
		{
			if (mMode != 1) return false;
			if (mInited == false) return false;
			if (mRoot == nullptr) return false;
			if (InElement == nullptr) return false;
			if (InElement->elem == nullptr) return false;

			InElement->elem->QueryDoubleText(&OutValue);
			return true;
		}
		//////////////////////////////////////////////////////////////////////////////
		bool SerializerXML::GetString(const Element* InElement, std::string& OutValue)
		{
			if (mMode != 1) return false;
			if (mInited == false) return false;
			if (mRoot == nullptr) return false;
			if (InElement == nullptr) return false;
			if (InElement->elem == nullptr) return false;

			OutValue = InElement->elem->GetText();
			return true;
		}
		//////////////////////////////////////////////////////////////////////////////
		bool SerializerXML::GetVector2(const Element* InElement, Vector2& OutValue, AttribVector2XML Attribs)
		{
			if (mMode != 1) return false;
			if (mInited == false) return false;
			if (mRoot == nullptr) return false;
			if (InElement == nullptr) return false;
			if (InElement->elem == nullptr) return false;

			InElement->elem->QueryFloatAttribute(Attribs.a.c_str(), &OutValue.X);
			InElement->elem->QueryFloatAttribute(Attribs.b.c_str(), &OutValue.Y);
			return true;
		}
		//////////////////////////////////////////////////////////////////////////////
		bool SerializerXML::GetVector3(const Element* InElement, Vector3& OutValue, AttribVector3XML Attribs)
		{
			if (mMode != 1) return false;
			if (mInited == false) return false;
			if (mRoot == nullptr) return false;
			if (InElement == nullptr) return false;
			if (InElement->elem == nullptr) return false;

			InElement->elem->QueryFloatAttribute(Attribs.a.c_str(), &OutValue.X);
			InElement->elem->QueryFloatAttribute(Attribs.b.c_str(), &OutValue.Y);
			InElement->elem->QueryFloatAttribute(Attribs.c.c_str(), &OutValue.Z);
			return true;
		}
		//////////////////////////////////////////////////////////////////////////////
		bool SerializerXML::GetVector4(const Element* InElement, Vector4& OutValue, AttribVector4XML Attribs)
		{
			if (mMode != 1) return false;
			if (mInited == false) return false;
			if (mRoot == nullptr) return false;
			if (InElement == nullptr) return false;
			if (InElement->elem == nullptr) return false;

			InElement->elem->QueryFloatAttribute(Attribs.a.c_str(), &OutValue.X);
			InElement->elem->QueryFloatAttribute(Attribs.b.c_str(), &OutValue.Y);
			InElement->elem->QueryFloatAttribute(Attribs.c.c_str(), &OutValue.Z);
			InElement->elem->QueryFloatAttribute(Attribs.d.c_str(), &OutValue.W);
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
			tinyxml2::XMLElement* subElement = mDoc.NewElement(end.c_str());

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
			tinyxml2::XMLElement* subElement = mDoc.NewElement(end.c_str());

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
			tinyxml2::XMLElement* subElement = mDoc.NewElement(end.c_str());

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
			tinyxml2::XMLElement* subElement = mDoc.NewElement(end.c_str());

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
			tinyxml2::XMLElement* subElement = mDoc.NewElement(end.c_str());

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
			tinyxml2::XMLElement* subElement = mDoc.NewElement(end.c_str());

			if (getElementFromHierarchy(aElement) == nullptr) return false;
			subElement->SetText(aValue.c_str());
			mTmp->InsertEndChild(subElement);
			return true;
		}
		//////////////////////////////////////////////////////////////////////////////
		bool SerializerXML::SetVector2(std::string aElement, Vector2 aValue, AttribVector2XML aAttribs)
		{
			if (mMode != 0) return false;
			if (mInited == false) return false;
			if (mRoot == nullptr) return false;

			mTmp = mDoc.NewElement(aElement.c_str());
			if (mTmp == nullptr) return false;
			mTmp->SetAttribute(aAttribs.a.c_str(), aValue.X);
			mTmp->SetAttribute(aAttribs.b.c_str(), aValue.Y);
			mRoot->InsertEndChild(mTmp);
			mTmp = nullptr;
			return true;
		}
		//////////////////////////////////////////////////////////////////////////////
		bool SerializerXML::SetSubVector2(std::vector<std::string> aElement, Vector2 aValue, AttribVector2XML aAttribs)
		{
			if (mMode != 0) return false;
			if (mInited == false) return false;
			if (mRoot == nullptr) return false;

			std::string end = aElement[aElement.size() - 1];
			tinyxml2::XMLElement* subElement = mDoc.NewElement(end.c_str());

			if (getElementFromHierarchy(aElement) == nullptr) return false;
			subElement->SetAttribute(aAttribs.a.c_str(), aValue.X);
			subElement->SetAttribute(aAttribs.b.c_str(), aValue.Y);
			mTmp->InsertEndChild(subElement);
			return true;
		}
		//////////////////////////////////////////////////////////////////////////////
		bool SerializerXML::SetVector3(std::string aElement, Vector3 aValue, AttribVector3XML aAttribs)
		{
			if (mMode != 0) return false;
			if (mInited == false) return false;
			if (mRoot == nullptr) return false;

			mTmp = mDoc.NewElement(aElement.c_str());
			if (mTmp == nullptr) return false;
			mTmp->SetAttribute(aAttribs.a.c_str(), aValue.X);
			mTmp->SetAttribute(aAttribs.b.c_str(), aValue.Y);
			mTmp->SetAttribute(aAttribs.c.c_str(), aValue.Z);
			mRoot->InsertEndChild(mTmp);
			mTmp = nullptr;
			return true;
		}
		//////////////////////////////////////////////////////////////////////////////
		bool SerializerXML::SetSubVector3(std::vector<std::string> aElement, Vector3 aValue, AttribVector3XML aAttribs)
		{
			if (mMode != 0) return false;
			if (mInited == false) return false;
			if (mRoot == nullptr) return false;

			std::string end = aElement[aElement.size() - 1];
			tinyxml2::XMLElement* subElement = mDoc.NewElement(end.c_str());

			if (getElementFromHierarchy(aElement) == nullptr) return false;
			subElement->SetAttribute(aAttribs.a.c_str(), aValue.X);
			subElement->SetAttribute(aAttribs.b.c_str(), aValue.Y);
			subElement->SetAttribute(aAttribs.c.c_str(), aValue.Z);
			mTmp->InsertEndChild(subElement);
			return true;
		}
		//////////////////////////////////////////////////////////////////////////////
		bool SerializerXML::SetVector4(std::string aElement, Vector4 aValue, AttribVector4XML aAttribs)
		{
			if (mMode != 0) return false;
			if (mInited == false) return false;
			if (mRoot == nullptr) return false;

			mTmp = mDoc.NewElement(aElement.c_str());
			if (mTmp == nullptr) return false;
			mTmp->SetAttribute(aAttribs.a.c_str(), aValue.X);
			mTmp->SetAttribute(aAttribs.b.c_str(), aValue.Y);
			mTmp->SetAttribute(aAttribs.c.c_str(), aValue.Z);
			mTmp->SetAttribute(aAttribs.d.c_str(), aValue.W);
			mRoot->InsertEndChild(mTmp);
			mTmp = nullptr;
			return true;
		}
		//////////////////////////////////////////////////////////////////////////////
		bool SerializerXML::SetSubVector4(std::vector<std::string> aElement, Vector4 aValue, AttribVector4XML aAttribs)
		{
			if (mMode != 0) return false;
			if (mInited == false) return false;
			if (mRoot == nullptr) return false;

			std::string end = aElement[aElement.size() - 1];
			tinyxml2::XMLElement* subElement = mDoc.NewElement(end.c_str());

			if (getElementFromHierarchy(aElement) == nullptr) return false;
			subElement->SetAttribute(aAttribs.a.c_str(), aValue.X);
			subElement->SetAttribute(aAttribs.b.c_str(), aValue.Y);
			subElement->SetAttribute(aAttribs.c.c_str(), aValue.Z);
			subElement->SetAttribute(aAttribs.d.c_str(), aValue.W);
			mTmp->InsertEndChild(subElement);
			return true;
		}
		//////////////////////////////////////////////////////////////////////////////
		bool SerializerXML::Save()
		{
			if (mMode != 0) return false;
			if (mInited == false) return false;
			if (mDoc.SaveFile(mFile.c_str()) != tinyxml2::XML_SUCCESS) return false;
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
			if (mDoc.LoadFile(mFile.c_str()) != tinyxml2::XML_SUCCESS) return false;
			mRoot = mDoc.FirstChildElement(mRootName.c_str());
			if (mRoot == nullptr) return false;
			mInited = true;
			return true;
		}
		//////////////////////////////////////////////////////////////////////////////
		bool SerializerXML::GetInt(std::string InElement, int& OutValue)
		{
			if (mMode != 1) return false;
			if (mInited == false) return false;
			if (mRoot == nullptr) return false;

			mTmp = mRoot->FirstChildElement(InElement.c_str());
			if (mTmp == nullptr) return false;
			mTmp->QueryIntText(&OutValue);
			return true;
		}
		//////////////////////////////////////////////////////////////////////////////
		bool SerializerXML::GetSubInt(std::vector<std::string> InElement, int& OutValue)
		{
			if (mMode != 1) return false;
			if (mInited == false) return false;
			if (mRoot == nullptr) return false;

			if (getElementFromHierarchy(InElement) == nullptr) return false;
			mTmp->QueryIntText(&OutValue);
			return true;
		}
		//////////////////////////////////////////////////////////////////////////////
		bool SerializerXML::GetBool(std::string InElement, bool& OutValue)
		{
			if (mMode != 1) return false;
			if (mInited == false) return false;
			if (mRoot == nullptr) return false;

			mTmp = mRoot->FirstChildElement(InElement.c_str());
			if (mTmp == nullptr) return false;
			mTmp->QueryBoolText(&OutValue);
			return true;
		}
		//////////////////////////////////////////////////////////////////////////////
		bool SerializerXML::GetSubBool(std::vector<std::string> InElement, bool& OutValue)
		{
			if (mMode != 1) return false;
			if (mInited == false) return false;
			if (mRoot == nullptr) return false;

			if (getElementFromHierarchy(InElement) == nullptr) return false;
			mTmp->QueryBoolText(&OutValue);
			return true;
		}
		//////////////////////////////////////////////////////////////////////////////
		bool SerializerXML::GetFloat(std::string InElement, float& OutValue)
		{
			if (mMode != 1) return false;
			if (mInited == false) return false;
			if (mRoot == nullptr) return false;

			mTmp = mRoot->FirstChildElement(InElement.c_str());
			if (mTmp == nullptr) return false;
			mTmp->QueryFloatText(&OutValue);
			return true;
		}
		//////////////////////////////////////////////////////////////////////////////
		bool SerializerXML::GetSubFloat(std::vector<std::string> InElement, float& OutValue)
		{
			if (mMode != 1) return false;
			if (mInited == false) return false;
			if (mRoot == nullptr) return false;

			if (getElementFromHierarchy(InElement) == nullptr) return false;
			mTmp->QueryFloatText(&OutValue);
			return true;
		}
		//////////////////////////////////////////////////////////////////////////////
		bool SerializerXML::GetDouble(std::string InElement, double& OutValue)
		{
			if (mMode != 1) return false;
			if (mInited == false) return false;
			if (mRoot == nullptr) return false;

			mTmp = mRoot->FirstChildElement(InElement.c_str());
			if (mTmp == nullptr) return false;
			mTmp->QueryDoubleText(&OutValue);
			return true;
		}
		//////////////////////////////////////////////////////////////////////////////
		bool SerializerXML::GetSubDouble(std::vector<std::string> InElement, double& OutValue)
		{
			if (mMode != 1) return false;
			if (mInited == false) return false;
			if (mRoot == nullptr) return false;

			if (getElementFromHierarchy(InElement) == nullptr) return false;
			mTmp->QueryDoubleText(&OutValue);
			return true;
		}
		//////////////////////////////////////////////////////////////////////////////
		bool SerializerXML::GetString(std::string InElement, std::string& OutValue)
		{
			if (mMode != 1) return false;
			if (mInited == false) return false;
			if (mRoot == nullptr) return false;

			mTmp = mRoot->FirstChildElement(InElement.c_str());
			if (mTmp == nullptr) return false;
			OutValue = mTmp->GetText();
			return true;
		}
		//////////////////////////////////////////////////////////////////////////////
		bool SerializerXML::GetSubString(std::vector<std::string> InElement, std::string& OutValue)
		{
			if (mMode != 1) return false;
			if (mInited == false) return false;
			if (mRoot == nullptr) return false;

			if (getElementFromHierarchy(InElement) == nullptr) return false;
			OutValue = mTmp->GetText();
			return true;
		}
		//////////////////////////////////////////////////////////////////////////////
		bool SerializerXML::GetVector2(std::string InElement, Vector2& OutValue, AttribVector2XML Attrib)
		{
			if (mMode != 1) return false;
			if (mInited == false) return false;
			if (mRoot == nullptr) return false;

			mTmp = mRoot->FirstChildElement(InElement.c_str());
			if (mTmp == nullptr) return false;
			mTmp->QueryFloatAttribute(Attrib.a.c_str(), &OutValue.X);
			mTmp->QueryFloatAttribute(Attrib.b.c_str(), &OutValue.Y);
			return true;
		}
		//////////////////////////////////////////////////////////////////////////////
		bool SerializerXML::GetSubVector2(std::vector<std::string> InElement, Vector2& OutValue, AttribVector2XML Attrib)
		{
			if (mMode != 1) return false;
			if (mInited == false) return false;
			if (mRoot == nullptr) return false;

			if (getElementFromHierarchy(InElement) == nullptr) return false;
			mTmp->QueryFloatAttribute(Attrib.a.c_str(), &OutValue.X);
			mTmp->QueryFloatAttribute(Attrib.b.c_str(), &OutValue.Y);
			return true;
		}
		//////////////////////////////////////////////////////////////////////////////
		bool SerializerXML::GetVector3(std::string InElement, Vector3& OutValue, AttribVector3XML Attrib)
		{
			if (mMode != 1) return false;
			if (mInited == false) return false;
			if (mRoot == nullptr) return false;

			mTmp = mRoot->FirstChildElement(InElement.c_str());
			if (mTmp == nullptr) return false;
			mTmp->QueryFloatAttribute(Attrib.a.c_str(), &OutValue.X);
			mTmp->QueryFloatAttribute(Attrib.b.c_str(), &OutValue.Y);
			mTmp->QueryFloatAttribute(Attrib.c.c_str(), &OutValue.Z);
			return true;
		}
		//////////////////////////////////////////////////////////////////////////////
		bool SerializerXML::GetSubVector3(std::vector<std::string> InElement, Vector3& OutValue, AttribVector3XML Attrib)
		{
			if (mMode != 1) return false;
			if (mInited == false) return false;
			if (mRoot == nullptr) return false;

			if (getElementFromHierarchy(InElement) == nullptr) return false;
			mTmp->QueryFloatAttribute(Attrib.a.c_str(), &OutValue.X);
			mTmp->QueryFloatAttribute(Attrib.b.c_str(), &OutValue.Y);
			mTmp->QueryFloatAttribute(Attrib.c.c_str(), &OutValue.Z);
			return true;
		}
		//////////////////////////////////////////////////////////////////////////////
		bool SerializerXML::GetVector4(std::string InElement, Vector4& OutValue, AttribVector4XML Attrib)
		{
			if (mMode != 1) return false;
			if (mInited == false) return false;
			if (mRoot == nullptr) return false;

			mTmp = mRoot->FirstChildElement(InElement.c_str());
			if (mTmp == nullptr) return false;
			mTmp->QueryFloatAttribute(Attrib.a.c_str(), &OutValue.X);
			mTmp->QueryFloatAttribute(Attrib.b.c_str(), &OutValue.Y);
			mTmp->QueryFloatAttribute(Attrib.c.c_str(), &OutValue.Z);
			mTmp->QueryFloatAttribute(Attrib.d.c_str(), &OutValue.W);
			return true;
		}
		//////////////////////////////////////////////////////////////////////////////
		bool SerializerXML::GetSubVector4(std::vector<std::string> InElement, Vector4& OutValue, AttribVector4XML Attrib)
		{
			if (mMode != 1) return false;
			if (mInited == false) return false;
			if (mRoot == nullptr) return false;

			if (getElementFromHierarchy(InElement) == nullptr) return false;
			mTmp->QueryFloatAttribute(Attrib.a.c_str(), &OutValue.X);
			mTmp->QueryFloatAttribute(Attrib.b.c_str(), &OutValue.Y);
			mTmp->QueryFloatAttribute(Attrib.c.c_str(), &OutValue.Z);
			mTmp->QueryFloatAttribute(Attrib.d.c_str(), &OutValue.W);
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
