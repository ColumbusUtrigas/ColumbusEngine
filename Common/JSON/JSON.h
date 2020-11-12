#pragma once

#include <Math/Vector2.h>
#include <Math/Vector3.h>
#include <Math/Vector4.h>
#include <Math/Quaternion.h>
#include <Core/ISerializable.h>
#include <Core/String.h>
#include <Core/Types.h>
#include <string_view>
#include <vector>
#include <map>

namespace Columbus
{

	class File;

	class JSON
	{
	public:
		enum class Type
		{
			String,
			Bool,
			Null,
			Int,
			Float,
			Array,
			Object
		};
	private:
		String StringValue;
		bool BoolValue = false;
		int64 IntValue = 0;
		double FloatValue = 0;
		std::vector<JSON> ArrayValue;
		std::map<String, JSON> ObjectValue;
		Type ValueType = Type::Object;

		bool IsVector = false;

		bool _ParseString(char*& Text, bool& Error);
		bool _ParseBool(char*& Text);
		bool _ParseNull(char*& Text);
		bool _ParseNumber(char*& Text, bool& Error);
		bool _ParseArray(char*& Text, bool& Error);
		bool _ParseObject(char*& Text, bool& Error);
		bool _Parse(char*& Text);
		void _WriteString(File& F) const;
		void _WriteBool(File& F) const;
		void _WriteNull(File& F) const;
		void _WriteInt(File& F) const;
		void _WriteFloat(File& F) const;
		void _WriteArray(File& F, uint32 Tabs) const;
		void _WriteObject(File& F, uint32 Tabs) const;
		void _Write(File& F, uint32 Tabs = 0) const;
	public:
		JSON() {}
		JSON(const String& String) : StringValue(String), ValueType(Type::String) {}
		JSON(const char* String) : StringValue(String), ValueType(Type::String) {}
		JSON(bool Bool) : BoolValue(Bool), ValueType(Type::Bool) {}
		JSON(std::nullptr_t) : ValueType(Type::Null) {}
		JSON(int Int) : IntValue(Int), ValueType(Type::Int) {}
		JSON(uint32 Int) : IntValue(Int), ValueType(Type::Int) {}
		JSON(int64 Int) : IntValue(Int), ValueType(Type::Int) {}
		JSON(float Float) : FloatValue(Float), ValueType(Type::Float) {}
		JSON(double Float) : FloatValue(Float), ValueType(Type::Float) {}

		JSON(const ISerializable* Ser)
		{
			if (Ser == nullptr)
			{
				*this = nullptr;
			}
			else
			{
				Ser->Serialize(*this);
			}
		}

		JSON(const ISerializable& Ser)
		{
			Ser.Serialize(*this);
		}

		template <typename T>
		JSON(const Vector2_t<T>& Vec)
		{
			IsVector = true;
			(*this)[0] = Vec.X;
			(*this)[1] = Vec.Y;
		}

		template <typename T>
		JSON(const Vector3_t<T>& Vec)
		{
			IsVector = true;
			(*this)[0] = Vec.X;
			(*this)[1] = Vec.Y;
			(*this)[2] = Vec.Z;
		}

		template <typename T>
		JSON(const Vector4_t<T>& Vec)
		{
			IsVector = true;
			(*this)[0] = Vec.X;
			(*this)[1] = Vec.Y;
			(*this)[2] = Vec.Z;
			(*this)[3] = Vec.W;
		}

		JSON(const Quaternion& Quat)
		{
			IsVector = true;
			(*this)[0] = Quat.X;
			(*this)[1] = Quat.Y;
			(*this)[2] = Quat.Z;
			(*this)[3] = Quat.W;
		}

		bool Parse(const char* Text);
		bool Load(const char* FileName);
		bool Save(const char* FileName);

		const String& GetString() const { return StringValue; }
		bool GetBool() const { return BoolValue; }
		int64 GetInt() const { return IntValue; }
		double GetFloat() const { return FloatValue; }
		template <typename T> Vector2_t<T> GetVector2() { return Vector2_t<T>((T)operator[](0).GetFloat(), (T)operator[](1).GetFloat()); }
		template <typename T> Vector3_t<T> GetVector3() { return Vector3_t<T>((T)operator[](0).GetFloat(), (T)operator[](1).GetFloat(), (T)operator[](2).GetFloat()); }
		template <typename T> Vector4_t<T> GetVector4() { return Vector4_t<T>((T)operator[](0).GetFloat(), (T)operator[](1).GetFloat(), (T)operator[](2).GetFloat(), (T)operator[](3).GetFloat()); }
		Quaternion GetQuaternion() { return Quaternion(operator[](0).GetFloat(), operator[](1).GetFloat(), operator[](2).GetFloat(), operator[](3).GetFloat()); }

		bool IsString() const { return ValueType == Type::String; }
		bool IsBool()   const { return ValueType == Type::Bool;   }
		bool IsNull()   const { return ValueType == Type::Null;   }
		bool IsInt()    const { return ValueType == Type::Int;    }
		bool IsFloat()  const { return ValueType == Type::Float;  }
		bool IsNumber() const { return IsInt() || IsFloat();      }
		bool IsArray()  const { return ValueType == Type::Array;  }
		bool IsObject() const { return ValueType == Type::Object; }

		size_t GetElementsCount() const { return  ArrayValue.size(); }
		size_t GetChildrenCount() const { return ObjectValue.size(); }
		bool HasChild(const String& Key) { return ObjectValue.find(Key) != ObjectValue.end(); }

		explicit operator bool() const { return GetBool(); }
		explicit operator int64() const { return GetInt(); }
		explicit operator int() const { return static_cast<int>(GetInt()); }
		explicit operator uint32() const { return static_cast<uint32>(GetInt()); }
		explicit operator double() const { return GetFloat(); }
		explicit operator float() const { return static_cast<float>(GetFloat()); }
		template <typename T> operator Vector2_t<T>() { return GetVector2<T>(); }
		template <typename T> operator Vector3_t<T>() { return GetVector3<T>(); }
		template <typename T> operator Vector4_t<T>() { return GetVector4<T>(); }
		operator Quaternion() { return GetQuaternion(); }

		JSON& operator[](size_t Index);
		JSON& operator[](std::string_view Key);

		bool operator==(const JSON& other) const;
		bool operator!=(const JSON& other) const;
	};


#define JSON_SERIALIZE_ENUM(ENUM, ...) \
	static void operator>>(JSON& j, ENUM& e) { \
		static_assert(std::is_enum<ENUM>::value, #ENUM " must be an enum!"); \
		static const std::pair<ENUM, JSON> m[] = __VA_ARGS__; \
		for (const auto& ej : m) { \
			if (ej.second == j) { \
				e = ej.first; \
				break; \
			} \
		} \
	} \
	static void operator<<(JSON& j, const ENUM e) {	\
		static_assert(std::is_enum<ENUM>::value, #ENUM " must be an enum!"); \
		static const std::pair<ENUM, JSON> m[] = __VA_ARGS__; \
		for (const auto& ej : m) { \
			if (ej.first == e) { \
				j = ej.second; \
				break; \
			} \
		} \
	}

}
