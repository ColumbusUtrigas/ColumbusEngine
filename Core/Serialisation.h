#pragma once

#include "Core/Reflection.h"
#include "Core/Guid.h"
using namespace Columbus;

#include "Lib/json/single_include/nlohmann/json.hpp"
#include <iostream>
#include <type_traits>

static void Reflection_SerialiseFieldJson(char* Object, const Reflection::Field& Field, nlohmann::json& json)
{
	char* FieldData = Object + Field.Offset;

	switch (Field.Type)
	{
	case Reflection::FieldType::Bool:
		json[Field.Name] = *(bool*)FieldData;
		break;
	case Reflection::FieldType::Int:
		json[Field.Name] = *(int*)FieldData;
		break;
	case Reflection::FieldType::Float:
		json[Field.Name] = *(float*)FieldData;
		break;
	case Reflection::FieldType::String:
		json[Field.Name] = *(std::string*)FieldData;
		break;
	case Reflection::FieldType::Enum:
		json[Field.Name] = *(int*)FieldData;
		break;
	case Reflection::FieldType::Struct:
	{
		auto& sjson = json[Field.Name];

		for (const auto& SField : Field.Struct->Fields)
		{
			Reflection_SerialiseFieldJson(FieldData, SField, sjson);
		}
		break;
	}
	case Reflection::FieldType::Array:
	{
		auto& sjson = json[Field.Name];
		sjson = nlohmann::json::array();

		std::vector<char>* ArrayData = (std::vector<char>*)FieldData;

		u32 ElementSize = Field.Array->ElementField.Size;
		u32 NumElements = ArrayData->size() / ElementSize;
		u32 Offset = 0;

		for (u32 i = 0; i < NumElements; i++)
		{
			nlohmann::json elementJson;
			Reflection_SerialiseFieldJson(ArrayData->data() + Offset, Field.Array->ElementField, elementJson);
			sjson.push_back(elementJson[Field.Array->ElementField.Name]); // strip the name

			Offset += ElementSize;
		}
		break;
	}

	case Reflection::FieldType::AssetRef:
	{
		struct AssetRefBase
		{
			std::string Path;
			// actual T* is templated, but we don't need it here, it's shady as fuck
		};

		auto* ref = (AssetRefBase*)FieldData;

		// GUID===Path format
		json[Field.Name] = std::string(Field.Typeguid) + "===" + ref->Path;
		break;
	}
	case Reflection::FieldType::ThingRef:
	{
		struct ThingRefBase
		{
			HGuid Guid;
			// actual T* is templated, but we don't need it here, it's shady
		};

		auto* ref = (ThingRefBase*)FieldData;

		// TypeGuid===ThingGuid format
		json[Field.Name] = std::string(Field.Typeguid) + "===" + std::to_string((u64)ref->Guid);
		break;
	}
	}
}

void Reflection_DeserialiseFieldInternalJson(char* FieldData, const Reflection::Field& Field, nlohmann::json& json);

static void Reflection_DeserialiseFieldJson(char* Object, const Reflection::Field& Field, nlohmann::json& json)
{
	char* FieldData = Object + Field.Offset;

	if (!json.contains(Field.Name))
	{
		Log::Warning("No field %s found", Field.Name);
		return;
	}

	Reflection_DeserialiseFieldInternalJson(FieldData, Field, json[Field.Name]);

	/*switch (Field.Type)
	{
	case Reflection::FieldType::Bool:
		*(bool*)FieldData = json[Field.Name];
		break;
	case Reflection::FieldType::Int:
		*(int*)FieldData = json[Field.Name];
		break;
	case Reflection::FieldType::Float:
		*(float*)FieldData = json[Field.Name];
		break;
	case Reflection::FieldType::String:
		*(std::string*)FieldData = json[Field.Name];
		break;
	case Reflection::FieldType::Enum:
		*(int*)FieldData = json[Field.Name];
		break;
	case Reflection::FieldType::Struct:
	{
		auto& sjson = json[Field.Name];

		for (const auto& SField : Field.Struct->Fields)
		{
			Reflection_DeserialiseFieldJson(FieldData, SField, sjson);
		}
		break;
	}

	case Reflection::FieldType::Array:
	{
		auto& sjson = json[Field.Name];
		if (!sjson.is_array())
		{
			Log::Error("Array field %s is not an array", Field.Name);
			break;
		}

		u32 ElementSize = Field.Array->ElementField.Size;
		u32 Offset = 0;

		for (auto& elementJson : sjson)
		{
			Field.Array->NewElement(FieldData);
			std::vector<char>* ArrayData = (std::vector<char>*)FieldData;

			Reflection_DeserialiseFieldJson(ArrayData->data() + Offset, Field.Array->ElementField, elementJson);

			Offset += ElementSize;
		}
		break;
	}

	case Reflection::FieldType::AssetRef:
	{
		auto& sjson = json[Field.Name];

		std::string refString = sjson.get<std::string>();
		size_t delim = refString.find("===");

		if (delim == std::string::npos)
		{
			Log::Error("AssetRef field %s is malformed: %s", Field.Name, refString.c_str());
			break;
		}

		std::string guid = refString.substr(0, delim);
		std::string path = refString.substr(delim + 3);

		if (guid != Field.Typeguid)
		{
			Log::Error("AssetRef type GUID mismatch for field %s\nExpected: %s\nFound:   %s",
				Field.Name, Field.Typeguid, guid.c_str());
			break;
		}

		// Assign to the actual AssetRef<T>::Path
		struct AssetRefBase {
			std::string Path;
			// pointer unused here
		};

		((AssetRefBase*)FieldData)->Path = path;
		break;
	}
	case Reflection::FieldType::ThingRef:
	{
		auto& sjson = json[Field.Name];

		std::string refString = sjson.get<std::string>();
		size_t delim = refString.find("===");
		if (delim == std::string::npos)
		{
			Log::Error("ThingRef field %s is malformed: %s", Field.Name, refString.c_str());
			break;
		}

		std::string guid = refString.substr(0, delim);
		std::string thingGuidStr = refString.substr(delim + 3);

		if (guid != Field.Typeguid)
		{
			Log::Error("ThingRef type GUID mismatch for field %s\nExpected: %s\nFound:   %s",
				Field.Name, Field.Typeguid, guid.c_str());
			break;
		}
		HGuid thingGuid = (HGuid)std::stoull(thingGuidStr);
		struct ThingRefBase {
			HGuid Guid;
			// pointer unused here
		};
		((ThingRefBase*)FieldData)->Guid = thingGuid;
		break;
	}

	}*/
}

static void Reflection_DeserialiseFieldInternalJson(char* FieldData, const Reflection::Field& Field, nlohmann::json& json)
{
	switch (Field.Type)
	{
	case Reflection::FieldType::Bool:
		*(bool*)FieldData = json;
		break;
	case Reflection::FieldType::Int:
		*(int*)FieldData = json;
		break;
	case Reflection::FieldType::Float:
		*(float*)FieldData = json;
		break;
	case Reflection::FieldType::String:
		*(std::string*)FieldData = json;
		break;
	case Reflection::FieldType::Enum:
		*(int*)FieldData = json;
		break;
	case Reflection::FieldType::Struct:
	{
		auto& sjson = json;

		for (const auto& SField : Field.Struct->Fields)
		{
			Reflection_DeserialiseFieldJson(FieldData, SField, sjson);
		}
		break;
	}

	case Reflection::FieldType::Array:
	{
		auto& sjson = json;
		if (!sjson.is_array())
		{
			Log::Error("Array field %s is not an array", Field.Name);
			break;
		}

		u32 ElementSize = Field.Array->ElementField.Size;
		u32 Offset = 0;

		for (auto& elementJson : sjson)
		{
			Field.Array->NewElement(FieldData);
			std::vector<char>* ArrayData = (std::vector<char>*)FieldData;

			Reflection_DeserialiseFieldInternalJson(ArrayData->data() + Offset, Field.Array->ElementField, elementJson);

			Offset += ElementSize;
		}
		break;
	}

	case Reflection::FieldType::AssetRef:
	{
		auto& sjson = json;

		std::string refString = sjson.get<std::string>();
		size_t delim = refString.find("===");

		if (delim == std::string::npos)
		{
			Log::Error("AssetRef field %s is malformed: %s", Field.Name, refString.c_str());
			break;
		}

		std::string guid = refString.substr(0, delim);
		std::string path = refString.substr(delim + 3);

		if (guid != Field.Typeguid)
		{
			Log::Error("AssetRef type GUID mismatch for field %s\nExpected: %s\nFound:   %s",
				Field.Name, Field.Typeguid, guid.c_str());
			break;
		}

		// Assign to the actual AssetRef<T>::Path
		struct AssetRefBase {
			std::string Path;
			// pointer unused here
		};

		((AssetRefBase*)FieldData)->Path = path;
		break;
	}
	case Reflection::FieldType::ThingRef:
	{
		auto& sjson = json;

		std::string refString = sjson.get<std::string>();
		size_t delim = refString.find("===");
		if (delim == std::string::npos)
		{
			Log::Error("ThingRef field %s is malformed: %s", Field.Name, refString.c_str());
			break;
		}

		std::string guid = refString.substr(0, delim);
		std::string thingGuidStr = refString.substr(delim + 3);

		if (guid != Field.Typeguid)
		{
			Log::Error("ThingRef type GUID mismatch for field %s\nExpected: %s\nFound:   %s",
				Field.Name, Field.Typeguid, guid.c_str());
			break;
		}
		HGuid thingGuid = (HGuid)std::stoull(thingGuidStr);
		struct ThingRefBase {
			HGuid Guid;
			// pointer unused here
		};
		((ThingRefBase*)FieldData)->Guid = thingGuid;
		break;
	}

	}
}


static void Reflection_SerialiseStructJson(char* Object, const Reflection::Struct* Struct, nlohmann::json& json, bool bWriteType = true)
{
	if (bWriteType)
	{
		json["0_type_guid"] = Struct->Guid;
		json["0_version"] = Struct->Version;
	}

	for (const auto& Field : Struct->Fields)
	{
		Reflection_SerialiseFieldJson(Object, Field, json);
	}
}

static void Reflection_DeserialiseStructJson(char* Object, const Reflection::Struct* Struct, nlohmann::json& json)
{
	for (const auto& Field : Struct->Fields)
	{
		Reflection_DeserialiseFieldJson(Object, Field, json);
	}
}

template <typename T>
static void Reflection_SerialiseStructJson(T& Object, nlohmann::json& json, bool bWriteType = true)
{
	Reflection_SerialiseStructJson((char*)&Object, Reflection::FindStructTypeForObject<T>(Object), json, bWriteType);
}

template <typename T>
static void Reflection_DeserialiseStructJson(T& Object, nlohmann::json& json)
{
	Reflection_DeserialiseStructJson((char*)&Object, Reflection::FindStructTypeForObject<T>(Object), json);
}

template <typename T>
static T* Reflection_DeserialiseStructJson_NewInstance(nlohmann::json& json)
{
	std::string Guid = json["0_type_guid"];
	const Reflection::Struct* Struct = Reflection::FindStructByGuid(Guid.c_str());

	T* NewInstance = (T*)Struct->Constructor();

	Reflection_DeserialiseStructJson((char*)NewInstance, Struct, json);

	return NewInstance;
}