#pragma once

#include <Core/SmartPointer.h>
#include <string>
#include <unordered_map>

namespace Columbus
{

	template <typename T>
	struct ResourceManager
	{
		uint32 CurrentID = 0;

		std::unordered_map<size_t, SmartPointer<T>> Resources;
		std::unordered_map<size_t, std::string> Names;
		std::unordered_map<std::string, size_t> IDs;

		bool IsNameFree(const std::string& Name)
		{
			return IDs.find(Name) == IDs.end();
		}

		bool Add(SmartPointer<T>&& Resource, const std::string& Name, bool Replace = false)
		{
			bool NameFree = IsNameFree(Name);

			if (NameFree || Replace)
			{
				Resources[CurrentID] = (SmartPointer<T>&&)std::move(Resource);
				Names[CurrentID] = Name;
				IDs[Name] = CurrentID;
				CurrentID++;
				return true;
			}

			return false;
		}

		T* Find(size_t ID)
		{
			auto It = Resources.find(ID);
			return It != Resources.end() ? It->second.Get() : nullptr;
		}
		
		T* Find(const std::string& Key)
		{
			auto It = IDs.find(Key);
			return It != IDs.end() ? Resources[It->second].Get() : nullptr;
		}

		std::string Find(const T* Value)
		{
			for (const auto& Elem : Resources)
			{
				if (Elem.second.Get() == Value)
				{
					return Names[Elem.first];
				}
			}

			return "";
		}

		size_t FindID(const std::string& Name)
		{
			auto It = IDs.find(Name);
			return It != IDs.end() ? It->second : 0;
		}

		SmartPointer<T>& operator[](const std::string& Key)
		{
			return Resources[IDs[Key]];
		}

		SmartPointer<T>& operator[](size_t ID)
		{
			return Resources[ID];
		}
	};

}


