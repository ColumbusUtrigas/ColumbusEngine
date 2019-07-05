#pragma once

#include <Core/SmartPointer.h>
#include <string>
#include <unordered_map>

namespace Columbus
{

	template <typename T>
	struct ResourceManager
	{
		std::unordered_map<std::string, SmartPointer<T>> Resources;

		bool IsNameFree(const std::string& Name)
		{
			return Resources.find(Name) == Resources.end();
		}

		bool Add(SmartPointer<T>&& Resource, const std::string& Name, bool Replace = false)
		{
			bool NameFree = IsNameFree(Name);

			if (NameFree || Replace)
			{
				Resources[Name] = (SmartPointer<T>&&)std::move(Resource);
				return true;
			}

			return false;
		}
		
		T* Find(const std::string& Key)
		{
			auto It = Resources.find(Key);
			return It != Resources.end() ? It->second.Get() : nullptr;
		}

		std::string Find(const T* Value)
		{
			for (const auto& Elem : Resources)
			{
				if (Elem.second.Get() == Value)
				{
					return Elem.first;
				}
			}

			return "";
		}

		SmartPointer<T>& operator[](const std::string& Key)
		{
			return Resources[Key];
		}
	};

}


