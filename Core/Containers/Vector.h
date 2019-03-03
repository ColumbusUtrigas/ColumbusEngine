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
		ValueType* _Data = nullptr;
		Allocator _Allocator;
		uint32 _Size = 0;
		uint32 _Capacity = 10;
	public:
		Vector()
		{
			_Data = _Allocator.Allocate(_Capacity);
		}
		Vector(const Vector& Base) { *this = Base; }
		Vector(Vector&& Base)      { *this = (Vector&&)Base; }

		uint32 Size()     const { return _Size;     }
		uint32 Capacity() const { return _Capacity; }

		ValueType* Data() const
		{
			return _Data;
		}

		void Clear()
		{
			//_Data = _Allocator.Reallocate(_Data, _Capacity);
			_Size = 0;
		}

		void Reserve(uint32 NewCapacity)
		{
			if (NewCapacity > _Capacity)
			{
				_Capacity = NewCapacity;
				_Data = _Allocator.Reallocate(_Data, _Capacity);
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

			new(&_Data[_Size++]) ValueType(std::forward<Args>(Arguments)...);
		}

		RandomAccessIterator<ValueType> begin() { return RandomAccessIterator<ValueType>(_Data); }
		RandomAccessIterator<ValueType> end() { return RandomAccessIterator<ValueType>(_Data + _Size); }

		const RandomAccessIterator<const ValueType> begin() const { return RandomAccessIterator<const ValueType>(_Data); }
		const RandomAccessIterator<const ValueType> end() const { return RandomAccessIterator<const ValueType>(_Data + _Size); }

		Vector& operator=(const Vector& Other)
		{
			_Size = Other._Size;
			_Capacity = Other._Capacity;
			_Allocator.Reallocate(_Data, _Capacity);
			memcpy(_Data, Other._Data, _Size * sizeof(ValueType)); // TODO
			return *this;
		}

		Vector& operator=(Vector&& Other)
		{
			_Size = Other._Size;         Other._Size = 0;
			_Capacity = Other._Capacity; Other._Capacity = 0;
			_Data = Other._Data;         Other._Data = nullptr;
			return *this;
		}

		ValueType& operator[](uint32 Index) const
		{
			return _Data[Index];
		}

		~Vector()
		{
			_Allocator.Deallocate(_Data);
		}
	};

}


