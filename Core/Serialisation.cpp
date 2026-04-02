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

	static bool ReflectionBinary_WriteString(Columbus::DataStream& Stream, const std::string& Value)
	{
		unsigned int Length = (unsigned int)Value.size();
		assert(Stream.Write(Length));
		if (Length > 0)
		{
			assert(Stream.WriteBytes(Value.data(), Length));
		}
		return true;
	}

	static bool ReflectionBinary_ReadString(Columbus::DataStream& Stream, std::string& OutValue)
	{
		unsigned int Length = 0;
		assert(Stream.Read(Length));

		OutValue.resize(Length);
		if (Length > 0)
		{
			assert(Stream.ReadBytes(OutValue.data(), Length));
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
			return Stream.Write(*(bool*)FieldData);
		case Reflection::FieldType::Int:
			return Stream.Write(*(int*)FieldData);
		case Reflection::FieldType::Float:
			return Stream.Write(*(float*)FieldData);
		case Reflection::FieldType::Enum:
			return Stream.Write(*(int*)FieldData);
		case Reflection::FieldType::String:
			return ReflectionBinary_WriteString(Stream, *(std::string*)FieldData);
		case Reflection::FieldType::Struct:
		{
			if (Field.Struct && Field.Struct->IsNativeBinary)
			{
				assert(Stream.WriteBytes(FieldData, (unsigned int)Field.Size));
				return true;
			}
			return Reflection_SerialiseStructBinary(Stream, FieldData, Field.Struct);
		}
		case Reflection::FieldType::Array:
		{
			std::vector<char>* ArrayData = (std::vector<char>*)FieldData;
			unsigned int ElementSize = (unsigned int)Field.Array->ElementField.Size;
			unsigned int ElementCount = ElementSize == 0 ? 0 : (unsigned int)(ArrayData->size() / ElementSize);
			assert(Stream.Write(ElementCount));

			if (ElementCount > 0 && ReflectionBinary_IsFieldNativeBinary(Field.Array->ElementField))
			{
				assert(Stream.WriteBytes(ArrayData->data(), ElementCount * ElementSize));
				return true;
			}

			for (unsigned int i = 0; i < ElementCount; ++i)
			{
				char* ElementPtr = ArrayData->data() + i * ElementSize;
				u64 PayloadHeaderOffset = Stream.Tell();
				unsigned int ElementPayloadSize = 0;
				assert(Stream.Write(ElementPayloadSize));

				u64 PayloadBegin = Stream.Tell();
				Reflection_SerialiseFieldBinaryPayload(Stream, ElementPtr, Field.Array->ElementField);

				ElementPayloadSize = (unsigned int)(Stream.Tell() - PayloadBegin);
				StreamPatchPod(Stream, PayloadHeaderOffset, ElementPayloadSize);
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
			return ReflectionBinary_WriteString(Stream, Field.Typeguid ? Field.Typeguid : "") && Stream.Write((u64)Ref->Guid);
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
			assert(PayloadSize == sizeof(bool));
			return Stream.Read(*(bool*)FieldData);
		case Reflection::FieldType::Int:
			assert(PayloadSize == sizeof(int));
			return Stream.Read(*(int*)FieldData);
		case Reflection::FieldType::Float:
			assert(PayloadSize == sizeof(float));
			return Stream.Read(*(float*)FieldData);
		case Reflection::FieldType::Enum:
			assert(PayloadSize == sizeof(int));
			return Stream.Read(*(int*)FieldData);
		case Reflection::FieldType::String:
			return ReflectionBinary_ReadString(Stream, *(std::string*)FieldData);
		case Reflection::FieldType::Struct:
			if (Field.Struct && Field.Struct->IsNativeBinary)
			{
				assert(PayloadSize == (unsigned int)Field.Size);
				assert(Stream.ReadBytes(FieldData, PayloadSize));
				return true;
			}
			return Reflection_DeserialiseStructBinaryPayload(Stream, FieldData, Field.Struct, PayloadSize);
		case Reflection::FieldType::Array:
		{
			Field.Array->Clear(FieldData);

			unsigned int ElementCount = 0;
			assert(Stream.Read(ElementCount));

			unsigned int ElementSize = (unsigned int)Field.Array->ElementField.Size;
			if (ElementCount > 0 && ReflectionBinary_IsFieldNativeBinary(Field.Array->ElementField))
			{
				std::vector<char>* ArrayData = (std::vector<char>*)FieldData;
				ArrayData->resize(ElementCount * ElementSize);
				assert(Stream.ReadBytes(ArrayData->data(), ElementCount * ElementSize));
			}
			else
			{
				for (unsigned int i = 0; i < ElementCount; ++i)
				{
					unsigned int ElementPayloadSize = 0;
					assert(Stream.Read(ElementPayloadSize));

					Field.Array->NewElement(FieldData);
					std::vector<char>* ArrayData = (std::vector<char>*)FieldData;
					char* ElementPtr = ArrayData->data() + i * ElementSize;

					Reflection_DeserialiseFieldBinaryPayload(Stream, ElementPtr, Field.Array->ElementField, ElementPayloadSize);
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
			ReflectionBinary_ReadString(Stream, TypeGuid);
			assert(Field.Typeguid == nullptr || TypeGuid == Field.Typeguid);

			return ReflectionBinary_ReadString(Stream, ((AssetRefBase*)FieldData)->Path);
		}
		case Reflection::FieldType::ThingRef:
		{
			struct ThingRefBase
			{
				HGuid Guid;
			};

			std::string TypeGuid;
			ReflectionBinary_ReadString(Stream, TypeGuid);
			assert(Field.Typeguid == nullptr || TypeGuid == Field.Typeguid);

			u64 Value = 0;
			assert(Stream.Read(Value));

			((ThingRefBase*)FieldData)->Guid = (HGuid)Value;
			break;
		}
		}

		assert(Stream.Tell() - PayloadBegin == PayloadSize);
		return true;
	}

	static bool Reflection_DeserialiseStructBinaryPayload(Columbus::DataStream& Stream, char* Object, const Reflection::Struct* Struct, unsigned int PayloadSize)
	{
		u64 PayloadBegin = Stream.Tell();

		ReflectionBinaryObjectHeader Header;
		assert(Stream.Read(Header));
		assert(Header.Magic == 'ROBJ');
		assert(Header.FormatVersion == 1);

		std::string StoredStructGuid;
		ReflectionBinary_ReadString(Stream, StoredStructGuid);
		assert(Struct->Guid == nullptr || StoredStructGuid == Struct->Guid);

		for (unsigned int i = 0; i < Header.FieldCount; ++i)
		{
			ReflectionBinaryFieldHeader StoredField;
			assert(Stream.Read(StoredField));

			std::string StoredFieldName;
			StoredFieldName.resize(StoredField.FieldNameLength);
			if (StoredField.FieldNameLength > 0)
			{
				assert(Stream.ReadBytes(StoredFieldName.data(), StoredField.FieldNameLength));
			}

			const Reflection::Field* Field = ReflectionBinary_FindFieldByName(Struct, StoredFieldName);
			if (Field == nullptr)
			{
				assert(StoredField.PayloadSize <= INT_MAX);
				Stream.SeekCur((i32)StoredField.PayloadSize);
				continue;
			}

			assert(StoredField.FieldType == (unsigned int)Field->Type);
			Reflection_DeserialiseFieldBinaryPayload(Stream, Object + Field->Offset, *Field, StoredField.PayloadSize);
		}

		assert(Stream.Tell() - PayloadBegin == PayloadSize);
		return true;
	}
}

bool Reflection_SerialiseStructBinary(DataStream& Stream, char* Object, const Reflection::Struct* Struct)
{
	if (!Stream.IsValid())
		return false;

	assert(Struct != nullptr);

	ReflectionBinaryObjectHeader Header;
	Header.TypeVersion = (unsigned int)Struct->Version;
	u64 HeaderOffset = Stream.Tell();
	assert(Stream.Write(Header));
	unsigned int FieldCount = 0;

	ReflectionBinary_WriteString(Stream, Struct->Guid ? Struct->Guid : "");

	for (const Reflection::Field& Field : Struct->Fields)
	{
		ReflectionBinaryFieldHeader HeaderField;
		HeaderField.FieldNameLength = (unsigned int)strlen(Field.Name);
		HeaderField.FieldType = (unsigned int)Field.Type;
		u64 FieldHeaderOffset = Stream.Tell();
		assert(Stream.Write(HeaderField));
		if (HeaderField.FieldNameLength > 0)
		{
			assert(Stream.WriteBytes(Field.Name, HeaderField.FieldNameLength));
		}

		u64 PayloadBegin = Stream.Tell();
		Reflection_SerialiseFieldBinaryPayload(Stream, Object + Field.Offset, Field);

		HeaderField.PayloadSize = (unsigned int)(Stream.Tell() - PayloadBegin);
		StreamPatchPod(Stream, FieldHeaderOffset, HeaderField);

		++FieldCount;
	}

	Header.FieldCount = FieldCount;
	StreamPatchPod(Stream, HeaderOffset, Header);
	return true;
}

bool Reflection_DeserialiseStructBinary(DataStream& Stream, char* Object, const Reflection::Struct* Struct)
{
	if (!Stream.IsValid())
		return false;

	assert(Struct != nullptr);
	return Reflection_DeserialiseStructBinaryPayload(Stream, Object, Struct, (unsigned int)(Stream.GetSize() - Stream.Tell()));
}
