#include <Core/String.h>
#include <ctype.h>
#include <stdio.h>
#include <limits.h>

namespace Columbus
{

	String::String()
	{
		reserve();
	}

	String::String(char c)
	{
		*this = c;
	}

	String::String(size_t n, char c)
	{
		reserve(n);
		if (n > 0)
			memset(_data, c, n);
		_length = n;
	}

	String::String(const char* str)
	{
		*this = str;
	}

	String::String(const String& str)
	{
		*this = str;
	}

	String& String::operator=(char c)
	{
		clear();
		reserve(1);
		_length = 1;
		_data[0] = c;
		return *this;
	}

	String& String::operator=(const char* str)
	{
		clear();
		size_t len = strlen(str);
		reserve(len);
		_length = len;
		if (_length > 0)
			memcpy(_data, str, _length);
		return *this;
	}

	String& String::operator=(const String& str)
	{
		clear();
		reserve(str.length());
		_length = str.length();
		if (_length > 0)
			memcpy(_data, str.data(), _length);
		return *this;
	}

	#define STRING_FROM(fmt, val) \
		String ret(32, '\0'); \
		snprintf(ret._data, 32, fmt, val); \
		return ret;

	String String::from(int val)                { STRING_FROM("%d",   val); }
	String String::from(long val)               { STRING_FROM("%li",  val); }
	String String::from(long long val)          { STRING_FROM("%lli", val); }
	String String::from(unsigned int val)       { STRING_FROM("%u",   val); }
	String String::from(unsigned long val)      { STRING_FROM("%lu",  val); }
	String String::from(unsigned long long val) { STRING_FROM("%llu", val); }
	String String::from(float val)              { STRING_FROM("%g",   val); }
	String String::from(double val)             { STRING_FROM("%g",   val); }

	void String::reserve(size_t n)
	{
		if ((n != npos && n > _capacity) || _capacity == 0)
		{
			_capacity = n;
			if (_capacity < 8)
				_capacity = 8;

			if (_data)
			{
				char* tmp = (char*)malloc(_capacity + 1);
				memset(tmp, 0, _capacity + 1);
				memcpy(tmp, _data, _length);
				free(_data);
				_data = tmp;
			} else
			{
				_data = (char*)malloc(_capacity + 1);
				memset(_data, 0, _capacity + 1);
				_length = 0;
			}
		}
	}

	void String::resize(size_t n, char c)
	{
		if (n < _length)
		{
			memset(_data + n, 0, _length - n);
			_length = n;
		} else if (n > _length)
		{
			reserve(n);
			for (size_t i = _length; i < n - _length; i++)
				_data[i] = c;
			_length = n;
		}
	}

	size_t String::find(char c, size_t pos) const noexcept
	{
		for (size_t i = pos; i < _length; i++)
			if (_data[i] == c) return i;

		return npos;
	}

	size_t String::find(const char* str, size_t pos) const noexcept
	{
		if (pos >= _length) return npos;
		char* ptr = strstr(_data + pos, str);
		return ptr != nullptr ? ptr - _data : npos;
	}

	size_t String::find(const String& str, size_t pos) const noexcept
	{
		return find(str.c_str(), pos);
	}

	size_t String::rfind(char c, size_t pos) const noexcept
	{
		if (pos == npos || pos >= _length) pos = _length - 1;
		while (pos != npos)
		{
			if (_data[pos] == c)
				return pos;
			--pos;
		}

		return npos;
	}

	size_t String::rfind(const char* str, size_t pos) const noexcept
	{
		if (pos == npos || pos >= _length) pos = _length - 1;

		size_t len = strlen(str);
		if (len == 0) return npos;

		size_t match = len;
		
		while (pos != npos)
		{
			if (_data[pos] == str[match - 1])
				match--;
			else
				match = len;

			if (match == 0)
				return pos;

			--pos;
		}

		return npos;
	}

	size_t String::rfind(const String& str, size_t pos) const noexcept
	{
		return rfind(str.c_str(), pos);
	}

	String String::substr(size_t pos, size_t len) const
	{
		if (pos == npos) return "";
		if (len == npos) len = _length - pos;

		String result(len, ' ');

		for (size_t i = pos, j = 0; i < pos + len; i++, j++)
			result._data[j] = _data[i];

		return result;
	}

	String String::tolower() const
	{
		String result = *this;

		for (size_t i = 0; i < _length; i++)
			result._data[i] = ::tolower(_data[i]);

		return result;
	}

	String String::toupper() const
	{
		String result = *this;

		for (size_t i = 0; i < _length; i++)
			result._data[i] = ::toupper(_data[i]);

		return result;
	}

	String& String::append(size_t n, char c)
	{
		reserve(_length + n);
		memset(_data + _length, c, n);
		_length += n;
		return *this;
	}

	String& String::append(const char* str)
	{
		size_t len = strlen(str);
		reserve(_length + len);
		if (len > 0)
			memcpy(_data + _length, str, len);
		_length += len;
		return *this;
	}

	String& String::append(const String& str)
	{
		return append(str.c_str());
	}

	String String::operator+(char c) const
	{
		String result = *this;
		result.append(1, c);
		return result;
	}

	String String::operator+(const char* str) const
	{
		String result = *this;
		result.append(str);
		return result;
	}

	String String::operator+(const String& str) const
	{
		String result = *this;
		result.append(str);
		return result;
	}

}


