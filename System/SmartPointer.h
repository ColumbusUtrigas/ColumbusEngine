/************************************************
*                 SmartPointer.h                *
*************************************************
*          This file is a part of:              *
*               COLUMBUS ENGINE                 *
*************************************************
*             Nikolay(Columbus) Red             *
*                   02.11.2017                  *
*************************************************/

#pragma once

namespace Columbus
{

	template <class T>
	class C_SmartPtr
	{
	private:
		T* mObj = nullptr;
	public:
		C_SmartPtr(T* aObj) :
			mObj(aObj)
		{}

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
			if (mObj != nullptr)
				delete mObj;
		}
	};

}





