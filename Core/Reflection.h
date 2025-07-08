#pragma once

#include <vector>
#include <functional>

// Engine's reflection system

// header file declaration macros

#define CREFLECT_DECLARE_ENUM(x, guid) \
template <> static const Reflection::Enum* Reflection::FindEnum<x>() { return Reflection::FindEnumByName(#x); } \
template <> static const char* Reflection::FindEnumGuid<x>() { return guid; } \
template <> static const char* Reflection::FindTypeGuid<x>() { return guid; } \
template <> void Reflection::EnforceTypeLinkage<x>();

#define CREFLECT_DECLARE_STRUCT(x, version, guid) \
template <> static const Reflection::Struct* Reflection::FindStruct<x>() { return Reflection::FindStructByName(#x); } \
template <> static const char* Reflection::FindStructName<x>() { return #x; } \
template <> static const char* Reflection::FindStructGuid<x>() { return guid; } \
template <> static const char* Reflection::FindTypeGuid<x>() { return guid; } \
template <> static const int Reflection::FindStructVersion<x>() { return version; } \
template <> void Reflection::EnforceTypeLinkage<x>();

#define CREFLECT_DECLARE_STRUCT_VIRTUAL(x, version, guid) \
CREFLECT_DECLARE_STRUCT(x, version, guid) \
template <> static constexpr const bool Reflection::FindTypeIsVirtual<x>() { return true; }

#define CREFLECT_DECLARE_STRUCT_WITH_PARENT(x, parent, version, guid) \
CREFLECT_DECLARE_STRUCT(x, version, guid) \
template <> static const Reflection::Struct* Reflection::FindStructParent<x>() { return Reflection::FindStruct<parent>(); } \
template <> static const char* Reflection::FindStructParentGuid<x>() { return Reflection::FindStructGuid<parent>(); }

#define CREFLECT_DECLARE_STRUCT_WITH_PARENT_VIRTUAL(x, parent, version, guid) \
CREFLECT_DECLARE_STRUCT_VIRTUAL(x, version, guid) \
template <> static const Reflection::Struct* Reflection::FindStructParent<x>() { return Reflection::FindStruct<parent>(); } \
template <> static const char* Reflection::FindStructParentGuid<x>() { return Reflection::FindStructGuid<parent>(); }

// declares static and virtual functions to get type info
#define CREFLECT_BODY_STRUCT_VIRTUAL(type) public: \
static const Reflection::Struct* GetTypeStatic(); \
virtual const Reflection::Struct* GetTypeVirtual() const;

// cpp part of CREFLECT_BODY_STRUCT_VIRTUAL
#define CREFLECT_DEFINE_VIRTUAL(type) \
const Reflection::Struct* type::GetTypeStatic() { return Reflection::FindStruct<type>(); } \
const Reflection::Struct* type::GetTypeVirtual() const { return Reflection::FindStruct<type>(); }

// manual mode macros
// these macros should be used once inside .cpp file

#define CREFLECT_TOKENPASTE(x, y) x ## y
#define CREFLECT_TOKENPASTE2(x, y) CREFLECT_TOKENPASTE(x, y)


#define CREFLECT_ENUM_BEGIN(x, meta) \
template <> void ::Reflection::EnforceTypeLinkage<x>() {} \
struct XXX_CReflection_Enum_Initialiser_##x { \
	XXX_CReflection_Enum_Initialiser_##x() { Reflection::Enum* Enum = Reflection::RegisterEnum(#x, Reflection::FindEnumGuid<x>());
#define CREFLECT_ENUM_FIELD(e, idx) \
	Enum->Fields.push_back(Reflection::EnumField{ #e, (int)e, idx });
#define CREFLECT_ENUM_END() } } CREFLECT_TOKENPASTE2(XXX_CReflection_Enum_Initialiser_Instance_##x, __LINE__);

#define CREFLECT_STRUCT_BEGIN_CONSTRUCTOR(x, constructorLambda, meta) \
template <> void ::Reflection::EnforceTypeLinkage<x>() {} \
struct XXX_CReflection_Struct_Initialiser_##x { \
XXX_CReflection_Struct_Initialiser_##x() {\
	using LocalStructType = x; \
	Reflection::Struct* Struct = Reflection::RegisterStruct<x>(); \
	Struct->Constructor = constructorLambda; \
	Struct->Destructor = [](void* Object) { delete (LocalStructType*)Object; }; \
	Struct->ParentGuid = Reflection::FindStructParentGuid<x>();

#define CREFLECT_STRUCT_BEGIN(x, meta) \
CREFLECT_STRUCT_BEGIN_CONSTRUCTOR(x, []() -> void* { return (void*) (new x()); }, meta)

#define CREFLECT_STRUCT_FIELD(type, name, meta) \
	Reflection::EnforceTypeLinkage<type>(); \
	Struct->LocalFields.push_back(Reflection::Field{ #name, #type, Reflection::FindTypeGuid<type>(), meta, offsetof(LocalStructType, name), sizeof(LocalStructType::name) });

#define CREFLECT_STRUCT_FIELD_ASSETREF(type, name, meta) \
	Reflection::EnforceTypeLinkage<type>(); \
	Struct->LocalFields.push_back(Reflection::Field{ #name, "AssetRef", Reflection::FindTypeGuid<type>(), meta, offsetof(LocalStructType, name), sizeof(LocalStructType::name) });

#define CREFLECT_STRUCT_END() } } CREFLECT_TOKENPASTE2(XXX_CReflection_Struct_Initialiser_Instance_##x, __LINE__);

// UI drawing specialisation for the struct
#define CREFLECT_STRUCT_CUSTOM_UI(x, function) struct XXX_CReflection_StructCustomUI_Initialiser_##x { \
	XXX_CReflection_StructCustomUI_Initialiser_##x() { \
		Reflection::RegisterStructCustomUI<x>(function); \
	} } CREFLECT_TOKENPASTE2(XXX_CReflection_StructCustomUI_Initialiser_##x, __LINE__);


// code generation macros, used only by header tool
// for regular c++ compiler they don't produce anything

#define CENUM()
#define CSTRUCT()
#define CFIELD()
#define CFUNC()

namespace Reflection
{
	struct EnumField;
	struct Enum;

	struct Field;
	struct Struct;

	using StructCustomUIFunc = std::function<bool(char* Object, const Field& aField, int Depth)>;
	using StructChangeNofifyFunc = std::function<void(char* Object)>;

	enum class FieldType
	{
		Bool,
		Int,
		Float,
		Array,
		String,

		Enum,
		Struct,

		AssetRef,
	};


	struct EnumField
	{
		const char* Name;
		int Value;
		int Index;
	};

	struct Enum
	{
		const char* Name;
		const char* Guid;
		std::vector<EnumField> Fields;

		const EnumField* FindFieldByValue(int Value) const;
	};

	struct Field
	{
		const char* Name;
		const char* Typename;
		const char* Typeguid;
		const char* Meta;
		int Offset;
		int Size;

		FieldType Type;
		Enum* Enum = nullptr;
		Struct* Struct = nullptr;
	};

	struct Struct
	{
		const char* Name;
		const char* Guid;
		int Version;
		std::vector<Field> Fields;
		std::vector<Field> LocalFields;

		std::function<void*()> Constructor;
		std::function<void(void*)> Destructor; // deletes as well

		const char* ParentGuid;
		Struct* Parent;
		std::vector<Struct*> Children;

		StructCustomUIFunc CustomUI;
		StructChangeNofifyFunc ChangeNotify;
	};

	const Enum* FindEnumByGuid(const char* Guid);
	const Enum* FindEnumByName(const char* Name);

	const Struct* FindStructByGuid(const char* Guid);
	const Struct* FindStructByName(const char* Name);


	const std::vector<Struct*>& GetAllStructs();
	const std::vector<Enum*>&   GetAllEnums();


	// Manual type register

	Enum*   RegisterEnum(const char* Name, const char* Guid);
	Struct* RegisterStruct(const char* Name, const char* Guid, const int Version);

	template <typename T>
	static Struct* RegisterStruct()
	{
		return RegisterStruct(FindStructName<T>(), FindStructGuid<T>(), FindStructVersion<T>());
	}

	void RegisterStructCustomUI(const char* Guid, StructCustomUIFunc Func);
	template <typename T>
	static void RegisterStructCustomUI(StructCustomUIFunc Func)
	{
		RegisterStructCustomUI(FindStructGuid<T>(), Func);
	}

	void SubscribeUiChangeNotify(const char* Guid, StructChangeNofifyFunc Func);
	template <typename T>
	static void SubscribeUiChangeNotify(StructChangeNofifyFunc Func)
	{
		SubscribeUiChangeNotify(FindStructGuid<T>(), Func);
	}


	// must be called only once on start
	void Initialise();



	template <typename T> static const char* FindTypeGuid() { return nullptr; }
	template <typename T> static constexpr const bool  FindTypeIsVirtual() { return false; }



	template <typename T> static const Enum* FindEnum() { return nullptr; }
	template <typename T> static const char* FindEnumGuid() { return nullptr; }



	template <typename T> static const Struct* FindStruct() { return nullptr; }
	template <typename T> static const Struct* FindStructParent() { return nullptr; }
	template <typename T> static const char*   FindStructName() { return nullptr; }
	template <typename T> static const char*   FindStructGuid() { return nullptr; }
	template <typename T> static const char*   FindStructParentGuid() { return nullptr; }
	template <typename T> static const int     FindStructVersion() { return -1; }

	// to make sure that compiler will not remove "unused" static initialisers
	template <typename T> void EnforceTypeLinkage();


	// finds type for both primitive and virtual structs
	template <typename T>
	static const Reflection::Struct* FindStructTypeForObject(T& Object)
	{
		if constexpr (Reflection::FindTypeIsVirtual<T>())
		{
			return (Reflection::Struct*)Object.GetTypeVirtual();
		}
		else
		{
			return (Reflection::Struct*)Reflection::FindStruct<T>();
		}
	}
}
