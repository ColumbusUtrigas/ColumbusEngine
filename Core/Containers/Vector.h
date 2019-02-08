#pragma once

#include <Core/Types.h>
#include <cstdlib>

namespace Columbus
{

	template <typename ValueType>
	class RandomAccessIterator
	{
	private:
		ValueType* _Values;
	public:
		RandomAccessIterator(ValueType* Values) : _Values(Values) {}

		RandomAccessIterator& operator++() { ++_Values; return *this; }
		ValueType& operator*() const { return *_Values; }
		bool operator==(const RandomAccessIterator& Other) const { return _Values == Other._Values; }
		bool operator!=(const RandomAccessIterator& Other) const { return _Values != Other._Values; }
	};

	template <typename T>
	class Vector
	{
	private:
		uint32 TypeSize = sizeof(T);

		T* Data = nullptr;
		uint32 _Size = 0;
		uint32 _Capacity = 10;
	public:
		Vector()
		{
			Data = (T*)malloc(_Capacity * TypeSize);
		}
		Vector(const Vector& Base) { *this = Base; }
		Vector(Vector&& Base) { *this = Base; }

		uint32 Size() const { return _Size; }
		uint32 Capacity() const { return _Capacity; }

		void Clear()
		{
			Data = (T*)realloc(Data, _Capacity * TypeSize);
			_Size = 0;
		}

		void Reserve(uint32 NewCapacity)
		{
			if (NewCapacity > _Capacity)
			{
				_Capacity = NewCapacity;
				T* New = (T*)malloc(_Capacity * TypeSize);
				memcpy(New, Data, _Size * TypeSize);
				if (Data != nullptr) free(Data);
				Data = New;
			}
		}

		void Add(const T& Value)
		{
			Emplace(Value);
		}

		void Add(T&& Value)
		{
			Emplace((T&&)Value);
		}

		template <typename... Args>
		void Emplace(Args&&... Arguments)
		{
			if (_Size >= _Capacity)
			{
				Reserve(_Capacity * 2);
			}

			new(&Data[_Size++]) T(std::forward<Args>(Arguments)...);
		}

		RandomAccessIterator<T> begin() { return RandomAccessIterator<T>(Data); }
		RandomAccessIterator<T> end() { return RandomAccessIterator<T>(Data + _Size); }

		const RandomAccessIterator<const T> begin() const { return RandomAccessIterator<const T>(Data); }
		const RandomAccessIterator<const T> end() const { return RandomAccessIterator<const T>(Data + _Size); }

		Vector& operator=(const Vector& Other)
		{
			if (Data != nullptr) free(Data);
			_Size = Other._Size;
			_Capacity = Other._Capacity;
			Data = (T*)malloc(_Capacity * TypeSize);
			memcpy(Data, Other.Data, _Size * TypeSize);
			return *this;
		}

		T& operator[](uint32 Index) const
		{
			return Data[Index];
		}

		~Vector()
		{
			if (Data != nullptr)
			{
				free(Data);
			}
		}
	};

}


