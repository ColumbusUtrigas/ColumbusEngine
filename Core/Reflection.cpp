#include "Reflection.h"
#include "Guid.h"
#include "Assert.h"

#include <string_view>
#include <unordered_map>

template <> void Reflection::EnforceTypeLinkage<bool>() {}
template <> void Reflection::EnforceTypeLinkage<int>() {}
template <> void Reflection::EnforceTypeLinkage<float>() {}
template <> void Reflection::EnforceTypeLinkage<std::string>() {}

namespace Reflection
{

	//////////////////////////////////////////////////////////////////////////////////////////////////
	// All the data

	struct ReflectionData
	{
		static ReflectionData& Instance()
		{
			static ReflectionData data;
			return data;
		}

		std::vector<Enum*> AllEnums;
		std::unordered_map<std::string_view, Enum*> NameToEnums;
		std::unordered_map<std::string_view, Enum*> GuidToEnums;

		std::vector<Struct*> AllStructs;
		std::unordered_map<std::string_view, Struct*> NameToStructs;
		std::unordered_map<std::string_view, Struct*> GuidToStructs;

		std::unordered_map<std::string_view, StructCustomUIFunc> GuidStructCustomUI;
		std::unordered_map<std::string_view, StructChangeNofifyFunc> GuidStructChangeNotify;
	};

	//////////////////////////////////////////////////////////////////////////////////////////////////

	const EnumField* Enum::FindFieldByValue(int Value) const
	{
		for (int i = 0; i < Fields.size(); i++)
		{
			if (Fields[i].Value == Value)
				return &Fields[i];
		}

		return nullptr;
	}


	//////////////////////////////////////////////////////////////////////////////////////////////////
	// Type search

	const Enum* FindEnumByName(const char* Name)
	{
		if (ReflectionData::Instance().NameToEnums.contains(Name))
			return ReflectionData::Instance().NameToEnums[Name];

		return nullptr;
	}

	const Struct* FindStructByGuid(const char* Guid)
	{
		if (ReflectionData::Instance().GuidToStructs.contains(Guid))
			return ReflectionData::Instance().GuidToStructs[Guid];

		return nullptr;
	}

	const Struct* FindStructByName(const char* Name)
	{
		if (ReflectionData::Instance().NameToStructs.contains(Name))
			return ReflectionData::Instance().NameToStructs[Name];

		return nullptr;
	}

	const std::vector<Struct*>& GetAllStructs()
	{
		return ReflectionData::Instance().AllStructs;
	}

	const std::vector<Enum*>& GetAllEnums()
	{
		return ReflectionData::Instance().AllEnums;
	}

	bool HasParentType(const Struct* Child, const Struct* Parent)
	{
		if (!Child || !Parent)
			return false;
		if (Child == Parent)
			return true;

		if (Child->ParentGuid)
		{
			const Struct* ParentStruct = FindStructByGuid(Child->ParentGuid);
			if (ParentStruct)
			{
				return HasParentType(ParentStruct, Parent);
			}
		}
		return false;
	}

	// Type search
	//////////////////////////////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////////////////////////////
	// Manual type register

	Enum* RegisterEnum(const char* Name, const char* Guid)
	{
		Enum* eenum = new Enum();
		eenum->Name = Name;

		ReflectionData::Instance().AllEnums.push_back(eenum);
		ReflectionData::Instance().NameToEnums[Name] = eenum;
		ReflectionData::Instance().GuidToEnums[Guid] = eenum;

		return eenum;
	}

	Struct* RegisterStruct(const char* Name, const char* Guid, const int Version)
	{
		Struct* sstruct = new Struct();
		sstruct->Guid = Guid;
		sstruct->Name = Name;
		sstruct->Version = Version;

		ReflectionData::Instance().AllStructs.push_back(sstruct);
		ReflectionData::Instance().GuidToStructs[sstruct->Guid] = sstruct;
		ReflectionData::Instance().NameToStructs[sstruct->Name] = sstruct;

		return sstruct;
	}

	void RegisterStructCustomUI(const char* Guid, StructCustomUIFunc Func)
	{
		ReflectionData::Instance().GuidStructCustomUI[Guid] = Func;
	}

	void SubscribeUiChangeNotify(const char* Guid, StructChangeNofifyFunc Func)
	{
		if (ReflectionData::Instance().GuidToStructs.contains(Guid))
		{
			ReflectionData::Instance().GuidToStructs[Guid]->ChangeNotify = Func;
		}
	}

	// Manual type register
	//////////////////////////////////////////////////////////////////////////////////////////////////

	void Initialise()
	{
		using namespace Columbus;

		auto& DataInstance = ReflectionData::Instance();

		Log::Initialization("Reflection: initialising %i structs...", (int)ReflectionData::Instance().AllStructs.size());

		for (Struct* sstruct : ReflectionData::Instance().AllStructs)
		{
			for (Field& field : sstruct->LocalFields)
			{
				bool IsBasicType = false;

				if (strcmp(field.Typename, "bool") == 0)
				{
					IsBasicType = true;
					field.Type = FieldType::Bool;
					COLUMBUS_ASSERT(field.Size == sizeof(bool));
				}
				else if (strcmp(field.Typename, "int") == 0)
				{
					IsBasicType = true;
					field.Type = FieldType::Int;
					COLUMBUS_ASSERT(field.Size == sizeof(int));
				}
				else if (strcmp(field.Typename, "float") == 0)
				{
					IsBasicType = true;
					field.Type = FieldType::Float;
					COLUMBUS_ASSERT(field.Size == sizeof(float));
				}
				else if (strcmp(field.Typename, "std::string") == 0)
				{
					IsBasicType = true;
					field.Type = FieldType::String;
					COLUMBUS_ASSERT(field.Size == sizeof(std::string));
				}
				else if (strcmp(field.Typename, "AssetRef") == 0)
				{
					IsBasicType = true;
					field.Type = FieldType::AssetRef;

					// asset ref is path string + pointer
					COLUMBUS_ASSERT(field.Size == sizeof(std::string) + sizeof(size_t));
				}
				else if (strcmp(field.Typename, "ThingRef") == 0)
				{
					IsBasicType = true;
					field.Type = FieldType::ThingRef;

					// thing ref is guid + pointer
					COLUMBUS_ASSERT(field.Size == sizeof(HGuid) + sizeof(size_t));
				}

				if (!IsBasicType)
				{
					if (field.Typeguid && ReflectionData::Instance().GuidToEnums.contains(field.Typeguid))
					{
						field.Type = FieldType::Enum;
						field.Enum = ReflectionData::Instance().GuidToEnums[field.Typeguid];
						COLUMBUS_ASSERT(field.Size == sizeof(int));
					}

					if (field.Typeguid && ReflectionData::Instance().GuidToStructs.contains(field.Typeguid))
					{
						field.Type = FieldType::Struct;
						field.Struct = ReflectionData::Instance().GuidToStructs[field.Typeguid];
					}
				}
			}
		}

		Log::Initialization("Reflection: initialising inheritance...");

		{
			std::vector<Struct*> TypeHierarchy;

			for (Struct* sstruct : ReflectionData::Instance().AllStructs)
			{
				TypeHierarchy.clear();

				if (sstruct->ParentGuid)
				{
					sstruct->Parent = (Struct*)FindStructByGuid(sstruct->ParentGuid);
					sstruct->Parent->Children.push_back(sstruct);
				}

				Struct* s = sstruct;
				while (s)
				{
					TypeHierarchy.push_back(s);
					s = s->ParentGuid ? (Struct*)FindStructByGuid(s->ParentGuid) : nullptr;
				}

				for (int i = (int)TypeHierarchy.size() - 1; i >= 0; i--)
				{
					for (const auto& F : TypeHierarchy[i]->LocalFields)
					{
						sstruct->Fields.push_back(F);
					}
				}
			}
		}

		Log::Initialization("Reflection: setting %i struct custom UIs...", (int)ReflectionData::Instance().GuidStructCustomUI.size());

		for (const auto [Guid, Func] : ReflectionData::Instance().GuidStructCustomUI)
		{
			if (ReflectionData::Instance().GuidToStructs.contains(Guid))
			{
				ReflectionData::Instance().GuidToStructs[Guid]->CustomUI = Func;
			}
		}

		auto inst = ReflectionData::Instance();
		int asd = 123;
	}


}