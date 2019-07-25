#pragma once

#include <Core/SmartPointer.h>
#include <Core/String.h>
#include <vector>
#include <unordered_map>
#include <algorithm>

namespace Columbus
{

	struct GameObjectsHolder
	{
		std::vector<SmartPointer<GameObject>> Resources;
		std::unordered_map<String, uint32> ResourcesMap;

		bool IsNameFree(const String& Name)
		{
			return ResourcesMap.find(Name) == ResourcesMap.end();
		}

		bool Add(SmartPointer<GameObject>&& Resource, const String& Name, bool Replace = false)
		{
			bool NameFree = IsNameFree(Name);

			if (NameFree || Replace)
			{
				if (NameFree)
				{
					Resources.emplace_back(std::move(Resource));
					ResourcesMap[Name] = Resources.size() - 1;
				}
				else
				{
					Resources[ResourcesMap[Name]] = (SmartPointer<GameObject>&&)std::move(Resource);
				}

				return true;
			}

			return false;
		}

		bool Delete(const String& Name)
		{
			auto It = ResourcesMap.find(Name);
			if (It != ResourcesMap.end())
			{
				size_t Index = It->second;
				auto Ptr = Resources[Index].Get();

				auto Pred = [&](const SmartPointer<GameObject>& A) { return A.Get() == Ptr; };
				auto Removed = std::remove_if(Resources.begin(), Resources.end(), Pred);

				Resources.erase(Removed, Resources.end());
				ResourcesMap.erase(Name);

				for (auto& Elem : ResourcesMap)
				{
					if (Elem.second > Index)
						Elem.second--;
				}

				return true;
			}

			return false;
		}

		bool Rename(const String& Old, const String& New)
		{
			uint32 Index = ResourcesMap[Old];

			ResourcesMap.erase(Old);
			ResourcesMap[New] = Index;

			return true;
		}

		GameObject* Find(const String& Index)
		{
			auto It = ResourcesMap.find(Index);
			return It != ResourcesMap.end() ? Resources[It->second].Get() : nullptr;
		}

		SmartPointer<GameObject>& operator[](size_t Index)
		{
			return Resources[Index];
		}

		SmartPointer<GameObject>& operator[](const String& Index)
		{
			return Resources[ResourcesMap[Index]];
		}
	};

}


