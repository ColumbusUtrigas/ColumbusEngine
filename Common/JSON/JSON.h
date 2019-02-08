#pragma once

#include <Math/Vector2.h>
#include <Math/Vector3.h>
#include <Math/Vector4.h>
#include <Core/Containers/Vector.h>
#include <Core/Types.h>
#include <string>
#include <unordered_map>

namespace Columbus
{

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
		std::string StringValue;
		bool BoolValue = false;
		int64 IntValue = 0;
		double FloatValue = 0;
		Vector<JSON> ArrayValue;
		std::unordered_map<std::string, JSON> ObjectValue;
		Type ValueType;

		bool _ParseString(char*& Text, bool& Error);
		bool _ParseBool(char*& Text);
		bool _ParseNull(char*& Text);
		bool _ParseNumber(char*& Text, bool& Error);
		bool _ParseArray(char*& Text, bool& Error);
		bool _ParseObject(char*& Text, bool& Error);
		bool _Parse(char*& Text);
	public:
		bool Parse(const char* Text);
		bool Load(const char* FileName);

		const std::string& GetString() const { return StringValue; }
		bool GetBool() const { return BoolValue; }
		int64 GetInt() const { return IntValue; }
		double GetFloat() const { return FloatValue; }
		template <typename T> Vector2_t<T> GetVector2() { return Vector2_t<T>((T)operator[](0).GetFloat(), (T)operator[](1).GetFloat()); }
		template <typename T> Vector3_t<T> GetVector3() { return Vector3_t<T>((T)operator[](0).GetFloat(), (T)operator[](1).GetFloat(), (T)operator[](2).GetFloat()); }
		template <typename T> Vector4_t<T> GetVector4() { return Vector4_t<T>((T)operator[](0).GetFloat(), (T)operator[](1).GetFloat(), (T)operator[](2).GetFloat(), (T)operator[](3).GetFloat()); }

		bool IsString() const { return ValueType == Type::String; }
		bool IsBool()   const { return ValueType == Type::Bool;   }
		bool IsNull()   const { return ValueType == Type::Null;   }
		bool IsInt()    const { return ValueType == Type::Int;    }
		bool IsFloat()  const { return ValueType == Type::Float;  }
		bool IsNumber() const { return IsInt() || IsFloat();      }
		bool IsArray()  const { return ValueType == Type::Array;  }
		bool IsObject() const { return ValueType == Type::Object; }

		uint32 GetElementsCount() const { return  ArrayValue.Size(); }
		uint32 GetChildrenCount() const { return ObjectValue.size(); }

		JSON& operator[](uint32 Index)
		{
			if (ValueType != Type::Array)
			{
				ObjectValue.clear();
				ValueType = Type::Array;
			}

			if (Index >= ArrayValue.Size())
			{
				ArrayValue.Add(JSON());
			}

			return ArrayValue[Index];
		}

		JSON& operator[](const std::string& Key)
		{
			if (ValueType != Type::Object)
			{
				ArrayValue.Clear();
				ValueType = Type::Object;
			}

			return ObjectValue[Key];
		}
	};

}


