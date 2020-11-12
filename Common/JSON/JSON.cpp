#include <Common/JSON/JSON.h>
#include <System/File.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

namespace Columbus
{

	static void SkipSpace(char*& Text)
	{
		while (isspace(*Text))
		{
			Text++;
		}
	}

	static bool ExtractString(char*& Text, String& Result)
	{
		while (*Text != '"')
		{
			if (*Text == '\0' || *Text == '\n' || *Text == '\r') return false;
			Result += *Text++;
		}

		return true;
	}

	static bool ExtractInteger(char*& Text, int64& Result)
	{
		Result = 0;
		while (isdigit(*Text))
		{
			if (*Text == '\0' || *Text == '\n' || *Text == '\r') return false;
			Result = Result * 10 + (*Text++ - '0');
		}

		return true;
	}

	static bool ExtractFract(char*& Text, double& Result)
	{
		Result = 0;
		double Factor = 0.1;
		while (isdigit(*Text))
		{
			if (*Text == '\0' || *Text == '\n' || *Text == '\r') return false;
			Result += (*Text++ - '0') * Factor;
			Factor *= 0.1;
		}

		return true;
	}

	static void WriteTabs(File& F, uint32 Tabs)
	{
		for (uint32 i = 0; i < Tabs; i++)
			F << '\t';
	}

	bool JSON::_ParseString(char*& Text, bool& Error)
	{
		if (*Text == '"')
		{
			Text++;
			if (!ExtractString(Text, StringValue)) { Error = true;  return true; }
			if (*Text != '"') { Error = true; return true; }
			Text++;
			ValueType = Type::String;
			return true;
		}

		return false;
	}

	bool JSON::_ParseBool(char*& Text)
	{
		bool True = memcmp(Text, "true", 4) == 0;
		bool False = memcmp(Text, "false", 5) == 0;

		if (True || False)
		{
			BoolValue = True;
			Text += True ? 4 : 5;
			ValueType = Type::Bool;
			return true;
		}

		return false;
	}

	bool JSON::_ParseNull(char*& Text)
	{
		if (memcmp(Text, "null", 4) == 0)
		{
			Text += 4;
			ValueType = Type::Null;
			return true;
		}

		return false;
	}

	bool JSON::_ParseNumber(char*& Text, bool& Error)
	{
		bool Negative = *Text == '-';

		if (isdigit(*Text) || Negative)
		{
			Text += Negative ? 1 : 0;
			if (!ExtractInteger(Text, IntValue)) { Error = true; return true; }

			double Fract = 0.0;
			int64 Exponent = 0;

			if (*Text == '.')
			{
				Text++;
				if (!ExtractFract(Text, Fract)) { Error = true; return true; }
			}

			if (*Text == 'e' || *Text == 'E')
			{
				Text++;
				bool NegativeExponent = *Text == '-';
				if (*Text == '-' || *Text == '+') Text++;
				if (!ExtractInteger(Text, Exponent)) { Error = true; return true; }
				if (NegativeExponent) Exponent *= -1;
			}

			if (Fract == 0.0)
			{
				FloatValue = IntValue * (Negative ? -1 : 1) * pow(10.0, Exponent);
				IntValue = (int64)(IntValue * (Negative ? -1 : 1) * pow(10.0, Exponent));
				ValueType = Type::Int;
			}
			else
			{
				FloatValue = ((double)(IntValue)+Fract) * (Negative ? -1 : 1) * pow(10.0, Exponent);
				IntValue = (int64)FloatValue;
				ValueType = Type::Float;
			}

			return true;
		}

		return false;
	}

	bool JSON::_ParseArray(char*& Text, bool& Error)
	{
		if (*Text == '[')
		{
			Text++;

			while (*Text != '\0')
			{
				SkipSpace(Text);
				if (*Text == ']' && ArrayValue.size() == 0)
				{
					Text++;
					ValueType = Type::Array;
					return true;
				}

				JSON New;
				if (!New._Parse(Text)) { Error = true; return true; }

				ValueType = Type::Array;
				ArrayValue.push_back(New);

				SkipSpace(Text);

				if (*Text == ']') { Text++; return true; }
				if (*Text != ',') { Error = true; return true; }
				Text++;
			}
		}

		return false;
	}

	bool JSON::_ParseObject(char*& Text, bool& Error)
	{
		if (*Text == '{')
		{
			Text++;

			while (*Text != '\0')
			{
				SkipSpace(Text);
				if (*Text == '}' && ObjectValue.size() == 0)
				{
					Text++;
					ValueType = Type::Object;
					return true;
				}

				if (*Text == '"')
				{
					Text++;
					String Name;
					if (!ExtractString(Text, Name)) { Error = true; return true; }

					if (*Text != '"') { Error = true; return true; }
					Text++;
					SkipSpace(Text);

					if (*Text != ':') { Error = true; return true; }
					Text++;
					SkipSpace(Text);

					JSON New;
					if (!New._Parse(Text)) { Error = true; return true; }

					ValueType = Type::Object;
					ObjectValue[Name] = New;
				}

				SkipSpace(Text);

				if (*Text == '}') { Text++; return true; }
				if (*Text != ',') { Error = true; return true; }
				Text++;
			}
		}

		return false;
	}

	bool JSON::_Parse(char*& Text)
	{
		SkipSpace(Text);

		bool Error = false;

		if (_ParseString(Text, Error)) return !Error;
		if (_ParseBool(Text)) return true;
		if (_ParseNull(Text)) return true;
		if (_ParseNumber(Text, Error)) return !Error;
		if (_ParseArray(Text, Error)) { if (Error) ArrayValue.clear();  return !Error; }
		if (_ParseObject(Text, Error)) { if (Error) ObjectValue.clear(); return !Error; }

		return false;
	}

	void JSON::_WriteString(File& F) const
	{
		F << '"' << StringValue.c_str() << '"';
	}

	void JSON::_WriteBool(File& F) const
	{
		F << (BoolValue ? "true" : "false");
	}

	void JSON::_WriteNull(File& F) const
	{
		F << "null";
	}

	void JSON::_WriteInt(File& F) const
	{
		F << std::to_string(IntValue).c_str();
	}

	void JSON::_WriteFloat(File& F) const
	{
		F << std::to_string(FloatValue).c_str();
	}

	void JSON::_WriteArray(File& F, uint32 Tabs) const
	{
		if (ArrayValue.empty())
		{
			F << "[]";
			return;
		}

		F << '[';
		if (!IsVector) F << '\n';
		Tabs++;

		uint32 Counter = 0;
		bool IsLast = false;

		for (const auto& Elem : ArrayValue)
		{
			if (!IsVector) WriteTabs(F, Tabs);

			Elem._Write(F, Tabs);

			IsLast = ++Counter == ArrayValue.size();

			if (!IsLast)
				F << ',';

			if (IsVector)
			{
				if (!IsLast)
				{
					F << ' ';
				}
			}
			else
			{
				F << '\n';
			}
		}

		Tabs--;
		if (!IsVector) WriteTabs(F, Tabs);
		F << ']';
	}

	void JSON::_WriteObject(File& F, uint32 Tabs) const
	{
		if (ObjectValue.empty())
		{
			F << "{}";
			return;
		}

		F << "{\n";
		Tabs++;

		uint32 Counter = 0;

		for (const auto& Elem : ObjectValue)
		{
			WriteTabs(F, Tabs);
			F << '"' << Elem.first.c_str() << "\": ";
			Elem.second._Write(F, Tabs);

			if (++Counter != ObjectValue.size())
				F << ',';

			F << '\n';
		}

		Tabs--;
		WriteTabs(F, Tabs);
		F << "}";
	}

	void JSON::_Write(File& F, uint32 Tabs) const
	{
		switch (ValueType)
		{
		case Type::String: _WriteString(F); break;
		case Type::Bool: _WriteBool(F); break;
		case Type::Null: _WriteNull(F); break;
		case Type::Int: _WriteInt(F); break;
		case Type::Float: _WriteFloat(F); break;
		case Type::Array: _WriteArray(F, Tabs); break;
		case Type::Object: _WriteObject(F, Tabs); break;
		}
	}

	bool JSON::Parse(const char* Text)
	{
		return _Parse((char*&)Text);
	}

	bool JSON::Load(const char* FileName)
	{
		File F(FileName, "rt");
		if (!F.IsOpened() || F.IsEOF()) return false;

		char* Text = new char[(uint32)F.GetSize() + 1];
		F.ReadBytes(Text, F.GetSize());
		Text[F.GetSize()] = '\0';

		bool Result = Parse(Text);
		delete[] Text;
		return Result;
	}

	bool JSON::Save(const char* FileName)
	{
		File F(FileName, "wt");
		if (!F.IsOpened()) return false;

		_Write(F);
		F << '\n';

		return true;
	}

	JSON& JSON::operator[](size_t Index)
	{
		if (ValueType != Type::Array)
		{
			ObjectValue.clear();
			ValueType = Type::Array;
		}

		while (Index >= ArrayValue.size())
		{
			ArrayValue.push_back(JSON());
		}

		return ArrayValue[Index];
	}

	JSON& JSON::operator[](std::string_view key)
	{
		if (ValueType != Type::Object)
		{
			ArrayValue.clear();
			ValueType = Type::Object;
		}

		return ObjectValue[key.data()];
	}

	bool JSON::operator==(const JSON& other) const
	{
		bool result = ValueType == other.ValueType;
		if (result)
		{
			switch (ValueType)
			{
			case Type::String: return StringValue == other.StringValue;
			case Type::Bool:   return BoolValue == other.BoolValue;
			case Type::Null:   return true;
			case Type::Int:    return IntValue == other.IntValue;
			case Type::Float:  return FloatValue == other.FloatValue;
			case Type::Array:  return std::equal(ArrayValue.begin(), ArrayValue.end(), other.ArrayValue.begin());
			case Type::Object: return std::equal(ObjectValue.begin(), ObjectValue.end(), other.ObjectValue.begin());
			}
		}
		return result;
	}

	bool JSON::operator!=(const JSON& other) const
	{
		return !(*this == other);
	}

}
