#pragma once

#include <vector>
#include <unordered_map>
#include <algorithm>

namespace Columbus
{

	struct GameObjectsHolder
	{
		std::vector<SmartPointer<GameObject>> Resources;
		std::unordered_map<std::string, uint32> ResourcesMap;

		bool IsNameFree(const std::string& Name)
		{
			return ResourcesMap.find(Name) == ResourcesMap.end();
		}

		bool Add(SmartPointer<GameObject>&& Resource, const std::string& Name, bool Replace = false)
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

		bool Delete(const std::string& Name)
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

		GameObject* Find(const std::string& Index)
		{
			auto It = ResourcesMap.find(Index);
			return It != ResourcesMap.end() ? Resources[It->second].Get() : nullptr;
		}

		SmartPointer<GameObject>& operator[](size_t Index)
		{
			return Resources[Index];
		}

		SmartPointer<GameObject>& operator[](const std::string& Index)
		{
			return Resources[ResourcesMap[Index]];
		}
	};

}


