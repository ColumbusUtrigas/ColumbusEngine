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
	class ResourceManager
	{
	private:
		std::map<unsigned int, SmartPtr<T>> mResources;
	public:
		ResourceManager()
		{}

		void add(unsigned int aID, T* aResource)
		{
			mResources.insert(std::pair<unsigned int, SmartPtr<T>>(aID, aResource));
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

		void remove(unsigned int aID)
		{
			mResources.erase(aID);
		}

		void clear()
		{
			mResources.clear();
		}

		~ResourceManager()
		{}
	};

}















