#pragma once

#include <memory>

template <typename T>
using UPtr = std::unique_ptr<T>;

template <typename T>
using SPtr = std::shared_ptr<T>;

namespace Columbus
{

	/**
	* Simple smart pointer realization, analogue of std::unique_ptr.
	*/
	template <class Type>
	class SmartPointer
	{
	private:
		Type* Obj = nullptr;
	public:
		SmartPointer() : Obj(nullptr) {}
		explicit SmartPointer(Type* Pointer) : Obj(Pointer) {}
		SmartPointer(const SmartPointer&) = delete;
		SmartPointer(SmartPointer&& Other)
		{
			Type* Tmp = Obj;
			Obj = Other.Obj;
			Other.Obj = Tmp;
		}

		SmartPointer& operator=(const SmartPointer&) = delete;
		SmartPointer& operator=(SmartPointer&& Other)
		{
			Type* Tmp = Obj;
			Obj = Other.Obj;
			Other.Obj = Tmp;
			return *this;
		}

		bool operator==(Type* Other) const
		{
			return Obj == Other;
		}

		bool operator!=(Type* Other) const
		{
			return Obj != Other;
		}

		Type* operator->() const
		{
			return const_cast<Type*>(Obj);
		}

		Type& operator*() const
		{
			return *Obj;
		}

		Type* Get() const
		{
			return const_cast<Type*>(Obj);
		}

		bool IsValid() const
		{
			return Obj != nullptr;
		}

		~SmartPointer()
		{
			delete Obj;
		}
	};

}


