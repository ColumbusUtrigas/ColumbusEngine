#pragma once

#include <utility>

namespace Columbus
{

	template <class T>
	class SmartPointer
	{
	private:
		T* mObj = nullptr;
	public:
		SmartPointer(const SmartPointer&) = delete;

		SmartPointer() :
			mObj(nullptr)
		{ }

		explicit SmartPointer(T* aObj) :
			mObj(aObj)
		{ }

		SmartPointer(SmartPointer&& aOther)
		{
			std::swap(mObj, aOther.mObj);
		}

		SmartPointer& operator=(const SmartPointer&) = delete;

		SmartPointer& operator=(SmartPointer&& aOther)
		{
			std::swap(mObj, aOther.mObj);
			return *this;
		}

		bool operator==(T* Other) const
		{
			return mObj == Other;
		}

		bool operator!=(T* Other) const
		{
			return mObj != Other;
		}

		T* operator->() const
		{
			return const_cast<T*>(mObj);
		}

		T& operator*() const
		{
			return *mObj;
		}

		T* get() const
		{
			return const_cast<T*>(mObj);
		}

		bool isValide() const
		{
			return mObj != nullptr;
		}

		~SmartPointer()
		{
			delete mObj;
		}
	};

}





