/************************************************
*                 SmartPointer.h                *
*************************************************
*          This file is a part of:              *
*               COLUMBUS ENGINE                 *
*************************************************
*             Nikolay(Columbus) Red             *
*                   02.11.2017                  *
*************************************************/

#include <utility>

#pragma once

namespace Columbus
{

	template <class T>
	class C_SmartPtr
	{
	private:
		T* mObj = nullptr;
	public:
		C_SmartPtr(const C_SmartPtr&) = delete;

		C_SmartPtr(T* aObj) :
			mObj(aObj)
		{}

		C_SmartPtr(C_SmartPtr&& aOther)
		{
			std::swap(mObj, aOther.mObj);
		}

		C_SmartPtr& operator=(const C_SmartPtr&) = delete;

		C_SmartPtr& operator=(C_SmartPtr&& aOther)
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

		~C_SmartPtr()
		{
			delete mObj;
		}
	};

}





