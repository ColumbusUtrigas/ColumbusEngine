/************************************************
*                 SmartPointer.h                *
*************************************************
*          This file is a part of:              *
*               COLUMBUS ENGINE                 *
*************************************************
*                Nika(Columbus) Red             *
*                   02.11.2017                  *
*************************************************/

#include <utility>

#pragma once

namespace Columbus
{

	template <class T>
	class SmartPtr
	{
	private:
		T* mObj = nullptr;
	public:
		SmartPtr(const SmartPtr&) = delete;

		SmartPtr(T* aObj) :
			mObj(aObj)
		{}

		SmartPtr(SmartPtr&& aOther)
		{
			std::swap(mObj, aOther.mObj);
		}

		SmartPtr& operator=(const SmartPtr&) = delete;

		SmartPtr& operator=(SmartPtr&& aOther)
		{
			std::swap(mObj, aOther.mObj);
			return *this;
		}

		T& operator->()
		{
			return *mObj;
		}

		T& operator*()
		{
			return *mObj;
		}

		T& get()
		{
			return *mObj;
		}

		bool isValide()
		{
			return mObj != nullptr;
		}

		~SmartPtr()
		{
			delete mObj;
		}
	};

}





