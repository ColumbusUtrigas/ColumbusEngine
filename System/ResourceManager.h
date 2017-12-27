/************************************************
*                ResourceManager.h              *
*************************************************
*          This file is a part of:              *
*               COLUMBUS ENGINE                 *
*************************************************
*                Nika(Columbus) Red             *
*                   27.12.2017                  *
*************************************************/
#pragma once

#include <System/SmartPointer.h>
#include <map>

namespace Columbus
{

	template <class T>
	class C_ResourceManager
	{
	private:
		std::map<unsigned int, C_SmartPtr<T>> mResources;
	public:
		C_ResourceManager()
		{}

		void add(unsigned int aID, T* aResource)
		{
			mResources.insert(std::pair<unsigned int, C_SmartPtr<T>>(aID, aResource));
		}

		T get(unsigned int aID)
		{
			T ret;
			try
			{
				ret = *mResources.at(aID);
			} catch (std::exception e) {}

			return ret;
		}

		bool remove(unsigned int aID)
		{
			mResources.erase(aID);
		}

		void clear()
		{
			mResources.clear();
		}

		~C_ResourceManager()
		{}
	};

}















