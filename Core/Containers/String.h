#pragma once

#include <Core/Types.h>
#include <Core/Containers/Array.h>
#include <cstring>

namespace Columbus
{

	class String
	{
	private:
		Array<char> Data;
	public:
		inline String() { };

		inline String(const String& Base)
		{
			Data = Base.Data;
		};

		inline String(String&& Base) noexcept
		{
			Data = std::move(Base.Data);
		};
		
		inline String(const char* InData)
		{
			Data.Append(InData, strlen(InData) + 1);
		}

		inline String(char Fill, uint32 Count)
		{
			Data.Init(Fill, Count);
			Data.Add('\0');
		}
		
		template <typename InputIterator>
		inline String(InputIterator First, InputIterator Last)
		{
			Append(First, Last);
		}
		/*
		* Return length of string
		*/
		inline uint32 Length() const
		{
			return Data.GetCount() - 1;
		}
		/*
		* Extend string by appending additional characters
		*/
		inline String& Append(const String& InString)
		{
			Data.Resize(Data.GetCount() - 2);
			Data.Append(*InString, InString.Length());
			Data.Add('\0');
			return *this;	
		}

		inline String& Append(String&& InString)
		{
			Data.Resize(Data.GetCount() - 2);
			Data.Append(*InString, InString.Length());
			Data.Add('\0');
			return *this;	
		}

		inline String& Append(const char* InString)
		{
			Data.Resize(Data.GetCount() - 2);
			Data.Append(InString, strlen(InString));
			Data.Add('\0');
			return *this;
		}

		inline String& Append(const char Character)
		{
			Data.Resize(Data.GetCount() - 2);
			Data.Add(Character);
			Data.Add('\0');
			return *this;
		}

		template <typename InputIterator>
		inline String& Append(InputIterator First, InputIterator Last)
		{
			if (Data.Last() == '\0')
			{
				Data.Resize(Data.GetCount() - 2);
			}

			while (First != Last)
			{
				Data.Add(*First++);
			}

			Data.Add('\0');

			return *this;
		}
		/*
		* Return C-string
		*/
		inline const char* operator*() const
		{
			return Data.GetData();
		}

		inline String& operator=(const String& Other)
		{
			Data = Other.Data;
			return *this;
		}

		inline String& operator=(String&& Other)
		{
			Data = std::move(Other.Data);
			return *this;
		}

		inline String& operator=(const char* Other)
		{
			Data.Clear();
			Data.Append(Other, strlen(Other) + 1);
			return *this;
		}

		inline String& operator=(const char Other)
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
		/*
		* For std::cout and std::fstream
		*/
		inline friend std::ostream& operator<<(std::ostream& Stream, const String& InString)
		{
			Stream << *InString;
			return Stream;
		}
		
		inline friend std::ostream& operator<<(std::ostream& Stream, String&& InString)
		{
			Stream << *InString;
			return Stream;
		}

		inline ~String() { }
	};

}

















