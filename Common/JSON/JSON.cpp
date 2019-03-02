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

	static bool ExtractString(char*& Text, std::string& Result)
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
				if (NegativeExponent) Text++;
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
				FloatValue = ((double)(IntValue) + Fract) * (Negative ? -1 : 1) * pow(10.0, Exponent);
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
				if (*Text == ']' && ArrayValue.size() == 0) { ValueType = Type::Array; return true; }

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
				if (*Text == '}' && ObjectValue.size() == 0) { ValueType = Type::Object; return true; }

				if (*Text == '"')
				{
					Text++;
					std::string Name;
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
		if (_ParseArray(Text, Error))  { if (Error) ArrayValue.clear();  return !Error; }
		if (_ParseObject(Text, Error)) { if (Error) ObjectValue.clear(); return !Error; }

		return false;
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

}


