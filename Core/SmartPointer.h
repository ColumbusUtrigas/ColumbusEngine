#pragma once

namespace Columbus
{

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
			Obj = Other.Obj;
			Other.Obj = nullptr;
		}

		SmartPointer& operator=(const SmartPointer&) = delete;
		SmartPointer& operator=(SmartPointer&& Other)
		{
			Obj = Other.Obj;
			Other.Obj = nullptr;
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


