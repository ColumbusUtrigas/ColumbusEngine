#pragma once

#include <Core/Types.h>
#include <cstdlib>
#include <cstring>
#include <new>

namespace Columbus
{

	template <typename ValueType>
	class DefaultAllocator
	{
	public:
		ValueType* Allocate(uint32 Count)
		{
			return (ValueType*)malloc(Count * sizeof(ValueType));
		}

		ValueType* Reallocate(ValueType* Pointer, uint32 Count)
		{
			return (ValueType*)realloc(Pointer, Count * sizeof(ValueType));
		}

		void Deallocate(ValueType* Pointer)
		{
			if (Pointer)
			{
				free(Pointer);
			}
		}
	};

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

	template <typename ValueType, typename Allocator = DefaultAllocator<ValueType>>
	class Vector
	{
	private:
		ValueType* Data = nullptr;
		Allocator _Allocator;
		uint32 _Size = 0;
		uint32 _Capacity = 10;
	public:
		Vector()
		{
			Data = _Allocator.Allocate(_Capacity);
		}
		Vector(const Vector& Base) { *this = Base; }
		Vector(Vector&& Base)      { *this = Base; }

		uint32 Size()     const { return _Size;     }
		uint32 Capacity() const { return _Capacity; }

		void Clear()
		{
			Data = _Allocator.Reallocate(Data, _Capacity);
			_Size = 0;
		}

		void Reserve(uint32 NewCapacity)
		{
			if (NewCapacity > _Capacity)
			{
				_Capacity = NewCapacity;
				Data = _Allocator.Reallocate(Data, _Capacity);
			}
		}

		void Add(const ValueType& Value)
		{
			Emplace(Value);
		}

		void Add(ValueType&& Value)
		{
			Emplace((ValueType&&)Value);
		}

		template <typename... Args>
		void Emplace(Args&&... Arguments)
		{
			if (_Size >= _Capacity)
			{
				Reserve(_Capacity * 2);
			}

			new(&Data[_Size++]) ValueType(std::forward<Args>(Arguments)...);
		}

		RandomAccessIterator<ValueType> begin() { return RandomAccessIterator<ValueType>(Data); }
		RandomAccessIterator<ValueType> end() { return RandomAccessIterator<ValueType>(Data + _Size); }

		const RandomAccessIterator<const ValueType> begin() const { return RandomAccessIterator<const ValueType>(Data); }
		const RandomAccessIterator<const ValueType> end() const { return RandomAccessIterator<const ValueType>(Data + _Size); }

		Vector& operator=(const Vector& Other)
		{
			if (Data != nullptr) free(Data);
			_Size = Other._Size;
			_Capacity = Other._Capacity;
			Data = (ValueType*)malloc(_Capacity * sizeof(ValueType));
			memcpy(Data, Other.Data, _Size * sizeof(ValueType));
			return *this;
		}

		ValueType& operator[](uint32 Index) const
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


