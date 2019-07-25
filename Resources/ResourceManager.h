#pragma once

#include <Core/SmartPointer.h>
#include <Core/String.h>
#include <unordered_map>

namespace Columbus
{

	template <typename T>
	struct ResourceManager
	{
		uint32 CurrentID = 0;

		std::unordered_map<size_t, SmartPointer<T>> Resources;
		std::unordered_map<size_t, String> Names;
		std::unordered_map<String, size_t> IDs;

		bool IsNameFree(const String& Name)
		{
			return IDs.find(Name) == IDs.end();
		}

		bool Add(SmartPointer<T>&& Resource, const String& Name, bool Replace = false)
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

		bool Rename(const String& Old, const String& New)
		{
			size_t ID = FindID(Old);

			if (ID != 0)
			{
				Names[ID] = New;

				IDs.erase(Old);
				IDs[New] = ID;

				return true;
			}

			return false;
		}

		T* Find(size_t ID)
		{
			auto It = Resources.find(ID);
			return It != Resources.end() ? It->second.Get() : nullptr;
		}
		
		T* Find(const String& Key)
		{
			auto It = IDs.find(Key);
			return It != IDs.end() ? Resources[It->second].Get() : nullptr;
		}

		String Find(const T* Value)
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

		size_t FindID(const String& Name)
		{
			auto It = IDs.find(Name);
			return It != IDs.end() ? It->second : 0;
		}

		SmartPointer<T>& operator[](const String& Key)
		{
			return Resources[IDs[Key]];
		}

		SmartPointer<T>& operator[](size_t ID)
		{
			return Resources[ID];
		}
	};

}


