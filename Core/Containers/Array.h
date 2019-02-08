#pragma once

#include <Core/Memory.h>
#include <algorithm>

namespace Columbus
{

	template <typename ContainerType, typename ElementType, typename IndexType>
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

	template <typename T>
	class Array
	{
	private:
		T* Ptr;
		uint32 ArrayCount;
		uint32 ArrayMax;
	public:
		Array() :
			ArrayCount(0),
			ArrayMax(8)
		{
			Ptr = (T*)Memory::Malloc(ArrayMax * sizeof(T));
		}

		Array(const Array& Base) :
			ArrayCount(Base.ArrayCount),
			ArrayMax(Base.ArrayMax)
		{
			Ptr = (T*)Memory::Malloc(Base.GetMax() * sizeof(T));
			std::copy(Base.Ptr, Base.Ptr + Base.GetCount(), Ptr);
		}
		/*
		* Add element in back of array
		* @param const T Value: Value which will added in back of array
		*/
		void Add(const T& Value)
		{
			if (ArrayCount >= ArrayMax)
			{
				T* TmpPtr = (T*)Memory::Malloc(ArrayMax * 2 * sizeof(T));
				memcpy(TmpPtr, Ptr, GetCount() * sizeof(T));
				Memory::Free(Ptr);
				Ptr = TmpPtr;
				ArrayMax *= 2;
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
		* Resize array to Count elements
		* It will extend array if Count > count of elements in array
		* It will reduce array if Count < count of elements in array
		*/
		void Resize(uint32 Count)
		{
			int64 PWRCount = Math::UpperPowerOf2(Count);
			T* TmpPtr = (T*)Memory::Malloc((uint32)PWRCount * sizeof(T));

			if (Count <= ArrayCount)
			{
				std::copy(Ptr, Ptr + Count, TmpPtr);
				ArrayCount = Count;
			}
			else
			{
				std::copy(Ptr, Ptr + GetMax(), TmpPtr);
			}

			Memory::Free(Ptr);
			Ptr = TmpPtr;
			ArrayMax = (uint32)PWRCount;
		}
		/*
		* Fill an array with a number of copies of an element
		* @param const T Value: The value to which the array will be populated
		* @param uint32 Num: Number of element copies
		*/
		void Init(const T& Value, uint32 Num)
		{
			Clear();

			if (Num > ArrayMax)
			{
				uint64 PWRCount = Math::UpperPowerOf2(Num);
				T* TmpPtr = (T*)Memory::Malloc((uint32)PWRCount * sizeof(T));
				std::copy(Ptr, Ptr + GetMax(), TmpPtr);
				Memory::Free(Ptr);
				Ptr = TmpPtr;
				ArrayMax = (uint32)PWRCount;
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
				T* TmpPtr = (T*)Memory::Malloc((uint32)PWRCount * sizeof(T));
				std::copy(Ptr, Ptr + GetMax(), TmpPtr);
				Memory::Free(Ptr);
				Ptr = TmpPtr;
				ArrayMax = (uint32)PWRCount;
			}

			std::copy(Data, Data + Num, Ptr + ArrayCount);
			ArrayCount += Num;
		}
		/*
		* Test if index is valid
		* @param uint32 Index: Index to test
		* @return bool: True if index is valid, else false
		*/
		inline bool IsValidIndex(uint32 Index) const
		{
			return Index >= 0 && Index < ArrayCount;
		}
		/*
		* Return size of inner type
		* @return uint32: Size of inner type
		*/
		inline uint32 GetTypeSize() const
		{
			return sizeof(T);
		}
		/*
		* Return count of elements
		* @return uint32: Count of elements in array
		*/
		inline uint32 GetCount() const
		{
			return ArrayCount;
		}
		/*
		* Return max count of elements
		* @return uint32: Max count of elements in array
		*/
		inline uint32 GetMax() const
		{
			return ArrayMax;
		}
		/*
		* Return data pointer of array
		* @return T*: Data pointer
		*/
		inline T* GetData() const
		{
			return Ptr;
		}
		/*
		* Clear array data and set defaults
		*/
		inline void Clear()
		{
			ArrayCount = 0;
			ArrayMax = 8;

			if (Ptr != nullptr)
			{
				Memory::Free(Ptr);
				Ptr = nullptr;
			}

			Ptr = (T*)Memory::Malloc(ArrayMax * sizeof(T));
		}

		inline Array& operator=(const Array& Other)
		{
			if (*this != Other)
			{
				if (Ptr != nullptr)
				{
					Memory::Free(Ptr);
				}

				Ptr = (T*)Memory::Malloc(Other.GetMax() * sizeof(T));
				std::copy(Other.GetData(), Other.GetData() + Other.GetCount(), Ptr);

				ArrayMax = Other.ArrayMax;
				ArrayCount = Other.ArrayCount;
			}

			return *this;
		}

		inline Array& operator=(Array&& Other)
		{
			if (*this != Other)
			{
				std::swap(Ptr, Other.Ptr);
				std::swap(ArrayCount, Other.ArrayCount);
				std::swap(ArrayMax, Other.ArrayMax);
			}

			return *this;
		}

		/*
		* Get array element by index
		*/
		inline T& operator[](uint32 Index) const
		{
			return Ptr[Index];
		}

		inline bool operator==(const Array& Other)
		{
			return GetCount() == Other.GetCount() && memcmp(GetData(), Other.GetData(), GetCount() * sizeof(T));
		}

		inline bool operator!=(const Array& Other)
		{
			return !(*this == Other);
		}

		typedef ArrayIndexedIterator<Array, T, uint32> Iterator;
		typedef ArrayIndexedIterator<const Array, const T, uint32> ConstIterator;

		Iterator CreateIterator() const
		{
			return Iterator(this);
		}

		ConstIterator CreateConstIterator() const
		{
			return ConstIterator(this);
		}

		inline Iterator begin() { return Iterator(this, 0); }
		inline Iterator end() { return Iterator(this, GetCount()); }
		inline ConstIterator begin() const { return ConstIterator(this, 0); }
		inline ConstIterator end() const { return ConstIterator(this, GetCount()); }

		~Array()
		{
			Clear();
		}
	};

}


















