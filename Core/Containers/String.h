#pragma once

#include <Core/Types.h>
#include <Core/Containers/Array.h>
#include <cstring>

namespace Columbus
{

	class String
	{
	private:
		char* Data = nullptr;
		uint32 Cap = 0;
		uint32 Len = 0;
	public:
		String() {}
		String(const char* Str) { operator=(Str); }

		String& operator=(const char* Str)
		{
			Clear();
			Cap = strlen(Str);
			Len = Cap;
			Data = new char[Len + 1];
			memcpy(Data, Str, Len + 1);
			return *this;
		}

		char& operator[](unsigned int ID)
		{
			return Data[ID];
		}

		const char* operator*() const
		{
			return Data != nullptr ? Data : "";
		}

		void Resize(unsigned int Size)
		{
			if (Size > Len)
			{
				char* New = new char[Size + 1];
				memset(New + Len, 0, Size - Len + 1);
				memcpy(New, Data, Len);
				delete[] Data;
				Data = New;
				Cap = Size;
			}

			if (Size < Len)
			{
				char* New = new char[Size + 1];
				memcpy(New, Data, Len);
				delete[] Data;
				Data = New;
				Cap = Len = Size;
			}
		}

		void Append(char Character)
		{
			if (Cap == Len)
			{
				Resize((unsigned int)(Len * 1.5 + 1));
			}

			Data[Len++] = Character;
		}

		void Clear()
		{
			delete[] Data; Data = nullptr; Cap = Len = 0;
		}

		uint32 Capacity() const
		{
			return Cap;
		}

		uint32 Length() const
		{
			return Len;
		}

		~String()
		{
			Clear();
		}
	};

	/*class String
	{
	private:
		Array<char> Data;
	public:
		String() { };

		String(const String& Base)
		{
			Data = Base.Data;
		}

		String(String&& Base) noexcept
		{
			Data = Base.Data;
		}
		
		String(const char* InData)
		{
			Data.Append(InData, strlen(InData) + 1);
		}

		String(char Fill, uint32 Count)
		{
			Data.Init(Fill, Count);
			Data.Add('\0');
		}
		
		template <typename InputIterator>
		String(InputIterator First, InputIterator Last)
		{
			Append(First, Last);
		}

		uint32 Length() const
		{
			return Data.GetCount() - 1;
		}
		
		String& Append(const String& InString)
		{
			if (Data.Last() == '\0')
			{
				Data.Resize(Data.GetCount() - 2);
			}

			Data.Append(*InString, InString.Length());
			Data.Add('\0');
			return *this;	
		}

		String& Append(String&& InString)
		{
			if (Data.GetCount() != 0)
			{
				if (Data.Last() == '\0')
				{
					Data.Resize(Data.GetCount() - 1);
				}
			}

			Data.Append(*InString, InString.Length());
			Data.Add('\0');
			return *this;	
		}

		String& Append(const char* InString)
		{
			if (Data.GetCount() != 0)
			{
				if (Data.Last() == '\0')
				{
					Data.Resize(Data.GetCount() - 1);
				}
			}

			Data.Append(InString, strlen(InString));
			Data.Add('\0');
			return *this;
		}

		String& Append(const char Character)
		{
			if (Data.GetCount() != 0)
			{
				if (Data.Last() == '\0')
				{
					Data.Resize(Data.GetCount() - 1);
				}
			}
			
			Data.Add(Character);
			Data.Add('\0');
			return *this;
		}

		template <typename InputIterator>
		String& Append(InputIterator First, InputIterator Last)
		{
			if (Data.GetCount() != 0)
			{
				if (Data.Last() == '\0')
				{
					Data.Resize(Data.GetCount() - 1);
				}
			}

			while (First != Last)
			{
				Data.Add(*First++);
			}

			Data.Add('\0');

			return *this;
		}

		void Clear()
		{
			Data.Clear();
		}
		
		const char* operator*() const
		{
			return Data.GetData();
		}

		String& operator=(const String& Other)
		{
			Data = Other.Data;
			return *this;
		}

		String& operator=(String&& Other)
		{
			Data = Other.Data;
			return *this;
		}

		String& operator=(const char* Other)
		{
			Data.Clear();
			Data.Append(Other, strlen(Other) + 1);
			return *this;
		}

		String& operator=(const char Other)
		{
			Data.Clear();
			Data.Add(Other);
			return *this;
		}

		inline String operator+(const String& Other)
		{
			return String(*this) += Other;
		}

		inline String operator+(String&& Other)
		{
			return String(*this) += Other;
		}

		inline String operator+(const char* Other)
		{
			return String(*this) += Other;
		}

		inline String operator+(const char Character)
		{
			return String(*this) += Character;
		}

		inline String& operator+=(const String& Other)
		{
			return Append(Other);
		}

		inline String& operator+=(String&& Other)
		{
			return Append(Other);
		}

		inline String& operator+=(const char* Other)
		{
			return Append(Other);
		}

		inline String& operator+=(const char Character)
		{
			return Append(Character);
		}

		inline ~String() { }
	};*/

}


