#include "Serialisation.h"
#include <climits>
#include <cstring>

namespace
{
	struct ReflectionBinaryObjectHeader
	{
		unsigned int Magic = 'ROBJ';
		unsigned int FormatVersion = 1;
		unsigned int TypeVersion = 0;
		unsigned int FieldCount = 0;
	};

	struct ReflectionBinaryFieldHeader
	{
		unsigned int FieldNameLength = 0;
		unsigned int FieldType = 0;
		unsigned int PayloadSize = 0;
	};

	template<typename T>
	static bool StreamPatchPod(Columbus::DataStream& Stream, u64 Offset, const T& Value)
	{
		u64 Current = Stream.Tell();
		Stream.SeekSet(Offset);
		bool Result = Stream.Write(Value);
		Stream.SeekSet(Current);
		assert(Result);
		return Result;
	}

#define REFLECTION_BINARY_ENSURE(Expression) \
	do \
	{ \
		const bool bReflectionBinaryOk = !!(Expression); \
		assert(bReflectionBinaryOk); \
		if (!bReflectionBinaryOk) return false; \
	} while (false)

	static bool ReflectionBinary_WriteString(Columbus::DataStream& Stream, const std::string& Value)
	{
		unsigned int Length = (unsigned int)Value.size();
		REFLECTION_BINARY_ENSURE(Stream.Write(Length));

		if (Length > 0)
		{
			REFLECTION_BINARY_ENSURE(Stream.WriteBytes(Value.data(), Length));
		}
		return true;
	}

	static bool ReflectionBinary_ReadString(Columbus::DataStream& Stream, std::string& OutValue)
	{
		unsigned int Length = 0;
		REFLECTION_BINARY_ENSURE(Stream.Read(Length));

		OutValue.resize(Length);
		if (Length > 0)
		{
			REFLECTION_BINARY_ENSURE(Stream.ReadBytes(OutValue.data(), Length));
		}

		return true;
	}

	static bool ReflectionBinary_IsFieldNativeBinary(const Reflection::Field& Field)
	{
		switch (Field.Type)
		{
		case Reflection::FieldType::Bool:
		case Reflection::FieldType::Int:
		case Reflection::FieldType::Float:
		case Reflection::FieldType::Enum:
			return true;
		case Reflection::FieldType::Struct:
			return Field.Struct && Field.Struct->IsNativeBinary;
		default:
			return false;
		}
	}

	static const Reflection::Field* ReflectionBinary_FindFieldByName(const Reflection::Struct* Struct, const std::string& FieldName)
	{
		for (const Reflection::Field& Field : Struct->Fields)
		{
			if (FieldName == Field.Name)
				return &Field;
		}

		return nullptr;
	}

	static bool Reflection_SerialiseFieldBinaryPayload(Columbus::DataStream& Stream, char* FieldData, const Reflection::Field& Field);
	static bool Reflection_DeserialiseStructBinaryPayload(Columbus::DataStream& Stream, char* Object, const Reflection::Struct* Struct, unsigned int PayloadSize);

	static bool Reflection_SerialiseFieldBinaryPayload(Columbus::DataStream& Stream, char* FieldData, const Reflection::Field& Field)
	{
		switch (Field.Type)
		{
		case Reflection::FieldType::Bool:
			REFLECTION_BINARY_ENSURE(Stream.Write(*(bool*)FieldData));
			return true;
		case Reflection::FieldType::Int:
			REFLECTION_BINARY_ENSURE(Stream.Write(*(int*)FieldData));
			return true;
		case Reflection::FieldType::Float:
			REFLECTION_BINARY_ENSURE(Stream.Write(*(float*)FieldData));
			return true;
		case Reflection::FieldType::Enum:
			REFLECTION_BINARY_ENSURE(Stream.Write(*(int*)FieldData));
			return true;
		case Reflection::FieldType::String:
			return ReflectionBinary_WriteString(Stream, *(std::string*)FieldData);
		case Reflection::FieldType::Blob:
		{
			Columbus::Blob& Blob = *(Columbus::Blob*)FieldData;
			unsigned int ByteCount = (unsigned int)Blob.Size();
			REFLECTION_BINARY_ENSURE(Stream.Write(ByteCount));

			if (ByteCount > 0)
			{
				REFLECTION_BINARY_ENSURE(Stream.WriteBytes(Blob.Data(), ByteCount));
			}
			return true;
		}
		case Reflection::FieldType::Struct:
		{
			if (Field.Struct && Field.Struct->IsNativeBinary)
			{
				REFLECTION_BINARY_ENSURE(Stream.WriteBytes(FieldData, (unsigned int)Field.Size));
				return true;
			}
			return Reflection_SerialiseStructBinary(Stream, FieldData, Field.Struct);
		}
		case Reflection::FieldType::Array:
		{
			std::vector<char>* ArrayData = (std::vector<char>*)FieldData;
			unsigned int ElementSize = (unsigned int)Field.Array->ElementField.Size;
			unsigned int ElementCount = ElementSize == 0 ? 0 : (unsigned int)(ArrayData->size() / ElementSize);
			REFLECTION_BINARY_ENSURE(Stream.Write(ElementCount));

			if (ElementCount > 0 && ReflectionBinary_IsFieldNativeBinary(Field.Array->ElementField))
			{
				REFLECTION_BINARY_ENSURE(Stream.WriteBytes(ArrayData->data(), ElementCount * ElementSize));
				return true;
			}

			for (unsigned int i = 0; i < ElementCount; ++i)
			{
				char* ElementPtr = ArrayData->data() + i * ElementSize;
				u64 PayloadHeaderOffset = Stream.Tell();
				unsigned int ElementPayloadSize = 0;
				REFLECTION_BINARY_ENSURE(Stream.Write(ElementPayloadSize));

				u64 PayloadBegin = Stream.Tell();
				if (!Reflection_SerialiseFieldBinaryPayload(Stream, ElementPtr, Field.Array->ElementField))
					return false;

				ElementPayloadSize = (unsigned int)(Stream.Tell() - PayloadBegin);
				if (!StreamPatchPod(Stream, PayloadHeaderOffset, ElementPayloadSize))
					return false;
			}
			return true;
		}
		case Reflection::FieldType::AssetRef:
		{
			struct AssetRefBase
			{
				std::string Path;
			};

			AssetRefBase* Ref = (AssetRefBase*)FieldData;
			return ReflectionBinary_WriteString(Stream, Field.Typeguid ? Field.Typeguid : "") && ReflectionBinary_WriteString(Stream, Ref->Path);
		}
		case Reflection::FieldType::ThingRef:
		{
			struct ThingRefBase
			{
				HGuid Guid;
			};

			ThingRefBase* Ref = (ThingRefBase*)FieldData;
			if (!ReflectionBinary_WriteString(Stream, Field.Typeguid ? Field.Typeguid : ""))
				return false;
			REFLECTION_BINARY_ENSURE(Stream.Write((u64)Ref->Guid));
			return true;
		}
		}

		assert(false);
		return false;
	}

	static bool Reflection_DeserialiseFieldBinaryPayload(Columbus::DataStream& Stream, char* FieldData, const Reflection::Field& Field, unsigned int PayloadSize)
	{
		u64 PayloadBegin = Stream.Tell();

		switch (Field.Type)
		{
		case Reflection::FieldType::Bool:
			REFLECTION_BINARY_ENSURE(PayloadSize == sizeof(bool));
			REFLECTION_BINARY_ENSURE(Stream.Read(*(bool*)FieldData));
			return true;
		case Reflection::FieldType::Int:
			REFLECTION_BINARY_ENSURE(PayloadSize == sizeof(int));
			REFLECTION_BINARY_ENSURE(Stream.Read(*(int*)FieldData));
			return true;
		case Reflection::FieldType::Float:
			REFLECTION_BINARY_ENSURE(PayloadSize == sizeof(float));
			REFLECTION_BINARY_ENSURE(Stream.Read(*(float*)FieldData));
			return true;
		case Reflection::FieldType::Enum:
			REFLECTION_BINARY_ENSURE(PayloadSize == sizeof(int));
			REFLECTION_BINARY_ENSURE(Stream.Read(*(int*)FieldData));
			return true;
		case Reflection::FieldType::String:
			return ReflectionBinary_ReadString(Stream, *(std::string*)FieldData);
		case Reflection::FieldType::Blob:
		{
			Columbus::Blob& Blob = *(Columbus::Blob*)FieldData;
			unsigned int ByteCount = 0;
			REFLECTION_BINARY_ENSURE(Stream.Read(ByteCount));

			Blob.Bytes.resize(ByteCount);
			if (ByteCount > 0)
			{
				REFLECTION_BINARY_ENSURE(Stream.ReadBytes(Blob.Data(), ByteCount));
			}
			return true;
		}
		case Reflection::FieldType::Struct:
			if (Field.Struct && Field.Struct->IsNativeBinary)
			{
				REFLECTION_BINARY_ENSURE(PayloadSize == (unsigned int)Field.Size);
				REFLECTION_BINARY_ENSURE(Stream.ReadBytes(FieldData, PayloadSize));
				return true;
			}
			return Reflection_DeserialiseStructBinaryPayload(Stream, FieldData, Field.Struct, PayloadSize);
		case Reflection::FieldType::Array:
		{
			Field.Array->Clear(FieldData);

			unsigned int ElementCount = 0;
			REFLECTION_BINARY_ENSURE(Stream.Read(ElementCount));

			unsigned int ElementSize = (unsigned int)Field.Array->ElementField.Size;
			if (ElementCount > 0 && ReflectionBinary_IsFieldNativeBinary(Field.Array->ElementField))
			{
				std::vector<char>* ArrayData = (std::vector<char>*)FieldData;
				ArrayData->resize(ElementCount * ElementSize);
				REFLECTION_BINARY_ENSURE(Stream.ReadBytes(ArrayData->data(), ElementCount * ElementSize));
			}
			else
			{
				for (unsigned int i = 0; i < ElementCount; ++i)
				{
					unsigned int ElementPayloadSize = 0;
					REFLECTION_BINARY_ENSURE(Stream.Read(ElementPayloadSize));

					Field.Array->NewElement(FieldData);
					std::vector<char>* ArrayData = (std::vector<char>*)FieldData;
					char* ElementPtr = ArrayData->data() + i * ElementSize;

					if (!Reflection_DeserialiseFieldBinaryPayload(Stream, ElementPtr, Field.Array->ElementField, ElementPayloadSize))
						return false;
				}
			}
			break;
		}
		case Reflection::FieldType::AssetRef:
		{
			struct AssetRefBase
			{
				std::string Path;
			};

			std::string TypeGuid;
			if (!ReflectionBinary_ReadString(Stream, TypeGuid))
				return false;
			REFLECTION_BINARY_ENSURE(Field.Typeguid == nullptr || TypeGuid == Field.Typeguid);

			return ReflectionBinary_ReadString(Stream, ((AssetRefBase*)FieldData)->Path);
		}
		case Reflection::FieldType::ThingRef:
		{
			struct ThingRefBase
			{
				HGuid Guid;
			};

			std::string TypeGuid;
			if (!ReflectionBinary_ReadString(Stream, TypeGuid))
				return false;
			REFLECTION_BINARY_ENSURE(Field.Typeguid == nullptr || TypeGuid == Field.Typeguid);

			u64 Value = 0;
			REFLECTION_BINARY_ENSURE(Stream.Read(Value));

			((ThingRefBase*)FieldData)->Guid = (HGuid)Value;
			break;
		}
		}

		REFLECTION_BINARY_ENSURE(Stream.Tell() - PayloadBegin == PayloadSize);
		return true;
	}

	static bool Reflection_DeserialiseStructBinaryPayload(Columbus::DataStream& Stream, char* Object, const Reflection::Struct* Struct, unsigned int PayloadSize)
	{
		u64 PayloadBegin = Stream.Tell();

		ReflectionBinaryObjectHeader Header;
		REFLECTION_BINARY_ENSURE(Stream.Read(Header));
		REFLECTION_BINARY_ENSURE(Header.Magic == 'ROBJ');
		REFLECTION_BINARY_ENSURE(Header.FormatVersion == 1);

		std::string StoredStructGuid;
		if (!ReflectionBinary_ReadString(Stream, StoredStructGuid))
			return false;
		REFLECTION_BINARY_ENSURE(Struct->Guid == nullptr || StoredStructGuid == Struct->Guid);

		for (unsigned int i = 0; i < Header.FieldCount; ++i)
		{
			ReflectionBinaryFieldHeader StoredField;
			REFLECTION_BINARY_ENSURE(Stream.Read(StoredField));

			std::string StoredFieldName;
			StoredFieldName.resize(StoredField.FieldNameLength);
			if (StoredField.FieldNameLength > 0)
			{
				REFLECTION_BINARY_ENSURE(Stream.ReadBytes(StoredFieldName.data(), StoredField.FieldNameLength));
			}

			const Reflection::Field* Field = ReflectionBinary_FindFieldByName(Struct, StoredFieldName);
			if (Field == nullptr)
			{
				REFLECTION_BINARY_ENSURE(StoredField.PayloadSize <= INT_MAX);
				Stream.SeekCur((i32)StoredField.PayloadSize);
				continue;
			}

			REFLECTION_BINARY_ENSURE(StoredField.FieldType == (unsigned int)Field->Type);
			if (!Reflection_DeserialiseFieldBinaryPayload(Stream, Object + Field->Offset, *Field, StoredField.PayloadSize))
				return false;
		}

		REFLECTION_BINARY_ENSURE(Stream.Tell() - PayloadBegin == PayloadSize);
		return true;
	}
}

bool Reflection_SerialiseStructBinary(DataStream& Stream, char* Object, const Reflection::Struct* Struct)
{
	if (!Stream.IsValid())
		return false;

	REFLECTION_BINARY_ENSURE(Struct != nullptr);

	ReflectionBinaryObjectHeader Header;
	Header.TypeVersion = (unsigned int)Struct->Version;
	u64 HeaderOffset = Stream.Tell();
	REFLECTION_BINARY_ENSURE(Stream.Write(Header));
	unsigned int FieldCount = 0;

	if (!ReflectionBinary_WriteString(Stream, Struct->Guid ? Struct->Guid : ""))
		return false;

	for (const Reflection::Field& Field : Struct->Fields)
	{
		ReflectionBinaryFieldHeader HeaderField;
		HeaderField.FieldNameLength = (unsigned int)strlen(Field.Name);
		HeaderField.FieldType = (unsigned int)Field.Type;
		u64 FieldHeaderOffset = Stream.Tell();
		REFLECTION_BINARY_ENSURE(Stream.Write(HeaderField));
		if (HeaderField.FieldNameLength > 0)
		{
			REFLECTION_BINARY_ENSURE(Stream.WriteBytes(Field.Name, HeaderField.FieldNameLength));
		}

		u64 PayloadBegin = Stream.Tell();
		if (!Reflection_SerialiseFieldBinaryPayload(Stream, Object + Field.Offset, Field))
			return false;

		HeaderField.PayloadSize = (unsigned int)(Stream.Tell() - PayloadBegin);
		if (!StreamPatchPod(Stream, FieldHeaderOffset, HeaderField))
			return false;

		++FieldCount;
	}

	Header.FieldCount = FieldCount;
	return StreamPatchPod(Stream, HeaderOffset, Header);
}

bool Reflection_DeserialiseStructBinary(DataStream& Stream, char* Object, const Reflection::Struct* Struct)
{
	if (!Stream.IsValid())
		return false;

	REFLECTION_BINARY_ENSURE(Struct != nullptr);
	return Reflection_DeserialiseStructBinaryPayload(Stream, Object, Struct, (unsigned int)(Stream.GetSize() - Stream.Tell()));
}

#undef REFLECTION_BINARY_ENSURE
