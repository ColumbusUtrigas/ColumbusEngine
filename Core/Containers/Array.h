#pragma once

#include <Core/Memory.h>
#include <Math/MathUtil.h>

namespace Columbus
{

	#define ARRAY_COUNT(x) (sizeof(x) / sizeof(x[0]))

	template<typename ContainerType, typename ElementType, typename IndexType>
	class ArrayIndexedIterator
	{
	private:
		ContainerType* Container;
		IndexType Index;
	public:
		ArrayIndexedIterator(ContainerType* InContainer, IndexType StartIndex = 0) :
			Container(InContainer),
			Index(StartIndex)
		{ }
		/*
		* Conversion operator to bool type
		*/
		FORCEINLINE operator bool() const
		{
			return Container->IsValidIndex(Index);
		}
		/*
		* Return index of current element
		* @return uint32: Index of current element
		*/
		uint32 GetIndex() const
		{
			return Index;
		}
		/*
		* Reset iterator to the first element
		*/
		void Reset()
		{
			Index = 0;
		}
		/*
		* Set iterator to end element
		*/
		void SetToEnd()
		{
			Index = Container->Count() - 1;
		}

		ArrayIndexedIterator& operator++()
		{
			++Index;
			return *this;
		}

		ArrayIndexedIterator operator++(int)
		{
			ArrayIndexedIterator Tmp(*this);
			++Index;
			return Tmp;
		}

		ArrayIndexedIterator& operator--()
		{
			--Index;
			return *this;
		}

		ArrayIndexedIterator operator--(int)
		{
			ArrayIndexedIterator Tmp(*this);
			++Index;
			return Tmp;
		}

		FORCEINLINE ElementType& operator*()
		{
			return (*Container)[Index];
		}

		FORCEINLINE ElementType* operator->()
		{
			return &(*Container)[Index];
		}

		FORCEINLINE friend bool operator==(const ArrayIndexedIterator& L, const ArrayIndexedIterator& R) { return L.Index == R.Index && L.Container == R.Container; }
		FORCEINLINE friend bool operator!=(const ArrayIndexedIterator& L, const ArrayIndexedIterator& R) { return L.Index != R.Index || L.Container != R.Container; }
	};

	template<typename T>
	class Array
	{
	private:
		T* Ptr;
		uint32 ArrayCount;
		uint32 ArrayMax;
		uint32 ArrayEndIndex;
	public:
		Array() :
			ArrayCount(0),
			ArrayMax(8),
			ArrayEndIndex(0)
		{
			Ptr = new T[ArrayMax];
		}

		Array(const Array& Base) :
			ArrayCount(Base.ArrayCount),
			ArrayMax(Base.ArrayMax),
			ArrayEndIndex(Base.ArrayEndIndex)
		{
			Ptr = new T[Base.GetMax()];
			Memory::Memcpy(Ptr, Base.Ptr, Base.GetMax() * Base.GetTypeSize());
		}
		/*
		* Add element in back of array
		* @param const T Value: Value which will added in back of array
		*/
		void Add(const T Value)
		{
			if (ArrayEndIndex >= ArrayMax)
			{
				T* TmpPtr = new T[ArrayMax];
				Memory::Memcpy(TmpPtr, Ptr, ArrayMax * GetTypeSize());
				ArrayMax *= 2;
				delete[] Ptr;
				Ptr = TmpPtr;
			}

			Ptr[ArrayCount++] = Value;
		}
		/*
		* Return first element in array
		* @return T: First element in array
		*/
		T Front() const
		{
			return Ptr[0];
		}
		/*
		* Return last element in array
		* @return T: Last element in array
		*/
		T Last() const
		{
			return Ptr[ArrayCount - 1];
		}
		/*
		*
		*/
		void Resize(uint32 Count)
		{
			int64 PWRCount = Math::UpperPowerOf2(Count);
			T* TmpPtr = new T[PWRCount];

			if (Count <= ArrayCount)
			{
				Memory::Memcpy(TmpPtr, Ptr, Count * GetTypeSize());
				ArrayCount = Count;
				ArrayEndIndex = Count;
			}
			else
			{
				Memory::Memcpy(TmpPtr, Ptr, ArrayMax * GetTypeSize());
			}

			ArrayMax = (uint32)PWRCount;
			delete[] Ptr;
			Ptr = TmpPtr;
		}
		/*
		* Fill an array with a number of copies of an element
		* @param const T Value: The value to which the array will be populated
		* @param uint32 Num: Number of element copies
		*/
		void Init(const T Value, uint32 Num)
		{
			Clear();

			if (Num > ArrayMax)
			{
				uint64 PWRCount = Math::UpperPowerOf2(Num);
				T* TmpPtr = new T[PWRCount];
				Memory::Memcpy(TmpPtr, Ptr, ArrayMax * GetTypeSize());
				ArrayMax = (uint32)PWRCount;
				delete[] Ptr;
				Ptr = TmpPtr;
			}

			for (uint32 i = 0; i < Num; i++)
			{
				Ptr[ArrayCount++] = Value;
			}
		}
		/*
		* Append row data pointer in the back of array
		* @param const T* Data: Data pointer
		* @param uint32 Num: Number of elements
		*/
		void Append(const T* Data, uint32 Num)
		{
			uint32 Pos = ArrayCount + Num;

			if (Pos > ArrayMax)
			{
				int64 PWRCount = Math::UpperPowerOf2(Num);
				T* TmpPtr = new T[PWRCount];
				Memory::Memcpy(TmpPtr, Ptr, ArrayMax * GetTypeSize());
				ArrayMax = (uint32)PWRCount;
				delete[] Ptr;
				Ptr = TmpPtr;
			}

			Memory::Memcpy(Ptr + ArrayCount, Data, Num * GetTypeSize());
			ArrayCount += Num;
		}
		/*
		* Test if index is valid
		* @param uint32 Index: Index to test
		* @return bool: True if index is valid, else false
		*/
		FORCEINLINE bool IsValidIndex(uint32 Index) const
		{
			return Index >= 0 && Index < ArrayCount;
		}
		/*
		* Return size of inner type
		* @return uint32: Size of inner type
		*/
		FORCEINLINE uint32 GetTypeSize() const
		{
			return sizeof(T);
		}
		/*
		* Return count of elements
		* @return uint32: Count of elements in array
		*/
		FORCEINLINE uint32 GetCount() const
		{
			return ArrayCount;
		}
		/*
		* Return max count of elements
		* @return uint32: Max count of elements in array
		*/
		FORCEINLINE uint32 GetMax() const
		{
			return ArrayMax;
		}
		/*
		* Return data pointer of array
		* @return T*: Data pointer
		*/
		FORCEINLINE T* GetData() const
		{
			return Ptr;
		}
		/*
		* Clear array data and set defaults
		*/
		FORCEINLINE void Clear()
		{
			ArrayCount = 0;
			ArrayMax = 8;
			ArrayEndIndex = 0;

			if (Ptr != nullptr)
			{
				delete[] Ptr;
				Ptr = nullptr;
			}

			Ptr = new T[ArrayMax];
		}

		FORCEINLINE Array& operator=(const Array& Other)
		{
			if (*this != Other)
			{
				if (Ptr != nullptr)
				{
					delete[] Ptr;
				}

				Ptr = new T[Other.GetMax()];
				Memory::Memcpy(Ptr, Other.GetData(), Other.GetCount() * Other.GetTypeSize());

				ArrayMax = Other.ArrayMax;
				ArrayCount = Other.ArrayCount;
				ArrayEndIndex = Other.ArrayEndIndex;
			}

			return *this;
		}

		FORCEINLINE Array& operator=(Array&& Other)
		{
			if (*this != Other)
			{
				if (Ptr != nullptr)
				{
					delete[] Ptr;
				}

				Ptr = new T[Other.GetMax()];
				Memory::Memmove(Ptr, Other.GetData(), Other.GetCount() * Other.GetTypeSize());

				ArrayMax = Other.ArrayMax;
				ArrayCount = Other.ArrayCount;
				ArrayEndIndex = Other.ArrayEndIndex;
			}

			return *this;
		}

		/*
		* Get array element by index
		*/
		FORCEINLINE T& operator[](uint32 Index)
		{
			return Ptr[Index];
		}

		FORCEINLINE bool operator==(const Array& Other)
		{
			return GetCount() == Other.GetCount() && Memory::Memcmp(GetData(), Other.GetData(), GetCount() * GetTypeSize()) == 0;
		}

		FORCEINLINE bool operator!=(const Array& Other)
		{
			return !(*this == Other);
		}

		typedef ArrayIndexedIterator<Array, T, uint32> Iterator;
		typedef ArrayIndexedIterator<const Array, const T, uint32> ConstIterator;

		Iterator CreateIterator()
		{
			return Iterator(this);
		}

		ConstIterator CreateConstIterator()
		{
			return ConstIterator(this);
		}

		FORCEINLINE Iterator begin() { return Iterator(this, 0); }
		FORCEINLINE Iterator end() { return Iterator(this, GetCount()); }

		~Array()
		{
			Clear();
		}
	};

}


















