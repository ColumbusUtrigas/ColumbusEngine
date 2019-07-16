#pragma once

#include <stdlib.h>
#include <string.h>

namespace Columbus
{

	class String
	{
	private:
		char* _data = nullptr;
		size_t _length = 0;
		size_t _capacity = 0;

		void _free_data()
		{
			if (_data)
			{
				free(_data);
				_data = nullptr;
			}

			_length = 0;
			_capacity = 0;
		}

		template <typename T, int step>
		struct _iterator
		{
		private:
			T* _current;
		public:
			_iterator(T* val) : _current(val) {}

			_iterator& operator++() { _current += step; return *this; }
			_iterator& operator++(int) { _current += step; return *this; }
			T& operator*() const { return *_current; }
			bool operator==(const _iterator& rhs) const { return _current == rhs._current; }
			bool operator!=(const _iterator& rhs) const { return _current != rhs._current; }
		};
	public:
		static constexpr size_t npos = -1;

		using iterator = _iterator<char, 1>;
		using const_iterator = _iterator<const char, 1>;
		using reverse_iterator = _iterator<char, -1>;
		using const_reverse_iterator = _iterator<const char, -1>;
	public:
		String();
		String(char c);
		String(size_t n, char c);
		String(const char* str);
		String(const String& str);

		String& operator=(char c);
		String& operator=(const char* str);
		String& operator=(const String& str);

		static String from(int val);
		static String from(long val);
		static String from(long long val);
		static String from(size_t val);
		static String from(float val);
		static String from(double val);

		void reserve(size_t n = 0);
		void resize(size_t n, char c = 0);

		iterator begin() noexcept { return iterator(_data); }
		const_iterator begin() const noexcept { return const_iterator(_data); }
		reverse_iterator rbegin() noexcept { return reverse_iterator(_data); }
		const_reverse_iterator rbegin() const noexcept { return const_reverse_iterator(_data); }

		iterator end() noexcept { return iterator(&_data[_length]); }
		const_iterator end() const noexcept { return const_iterator(&_data[_length]); }
		reverse_iterator rend() noexcept { return reverse_iterator(&_data[_length]); }
		const_reverse_iterator rend() const noexcept { return const_reverse_iterator(&_data[_length]); }

		size_t size() const noexcept { return _length; }
		size_t length() const noexcept { return _length; }
		size_t capacity() const noexcept { return _capacity; }
		void clear() noexcept { if (_length > 0) memset(_data, 0, _length); _length = 0; }
		bool empty() const noexcept { return _length == 0; }

		const char* c_str() const noexcept { return _data; }
		const char* data() const noexcept { return _data; }

		size_t find(char c, size_t pos = 0) const noexcept;
		size_t find(const char* str, size_t pos = 0) const noexcept;
		size_t find(const String& str, size_t pos = 0) const noexcept;

		size_t rfind(char c, size_t pos = npos) const noexcept;
		size_t rfind(const char* str, size_t pos = npos) const noexcept;
		size_t rfind(const String& str, size_t pos = npos) const noexcept;

		char& front() { return _data[0]; }
		const char& front() const { return _data[0]; }

		char& back() { return _data[_length - 1]; }
		const char& back() const { return _data[_length - 1]; }

		char& operator[](size_t pos) { return _data[pos]; }
		const char& operator[](size_t pos) const { return _data[pos]; }

		void push_back(char c) { append(1, c); }
		void pop_back() { resize(_length - 1); }

		String substr(size_t pos = 0, size_t len = npos) const;
		String tolower() const;
		String toupper() const;

		String& append(size_t n, char c);
		String& append(const char* str);
		String& append(const String& str);

		String operator+(char c) const;
		String operator+(const char* str) const;
		String operator+(const String& str) const;

		friend String operator+(char lhs, const String& rhs) { return String(lhs) + rhs; }
		friend String operator+(const char* lhs, const String& rhs) { return String(lhs) + rhs; }

		String& operator+=(char c) { return append(1, c); }
		String& operator+=(const char* str) { return append(str); }
		String& operator+=(const String& str) { return append(str); }

		bool operator==(const char* str) const noexcept { return strcmp(_data, str) == 0; }
		bool operator==(const String& str) const noexcept { return *this == str.c_str(); }

		bool operator!=(const char* str) const noexcept { return !(*this == str); }
		bool operator!=(const String& str) const noexcept { return !(*this == str); }

		bool operator<(const char* str) const noexcept { return strcmp(_data, str) < 0; }
		bool operator<(const String& str) const noexcept { return *this < str.c_str(); }

		bool operator<=(const char* str) const noexcept { return *this < str || *this == str; }
		bool operator<=(const String& str) const noexcept { return *this < str || *this == str; }

		bool operator>(const char* str) const noexcept { return strcmp(_data, str) > 0; }
		bool operator>(const String& str) const noexcept { return *this > str.c_str(); }

		bool operator>=(const char* str) const noexcept { return *this > str || *this == str; }
		bool operator>=(const String& str) const noexcept { return *this > str || *this == str; }

		~String()
		{
			_free_data();
		}
	};

}

namespace std
{

	template <typename T> struct hash;

	template <>
	struct hash<Columbus::String>
	{
		size_t operator()(const Columbus::String& str) const
		{
			size_t hash = 0;
			const char* s = str.c_str();

			for (size_t i = 0; i < str.length(); i++)
			{
				hash = s[i] + (hash << 6) + (hash << 16) - hash;
			}

			return hash;
		}
	};

}


