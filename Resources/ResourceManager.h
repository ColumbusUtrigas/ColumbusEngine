#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>

namespace Columbus
{

	template <typename T>
	struct ResourceManager
	{
		// Resources map countains pairs of [resource_name; index in Resources]
		std::vector<SmartPointer<T>> Resources;
		std::unordered_map<std::string, size_t> ResourcesMap;

		bool IsNameFree(const std::string& Name)
		{
			return ResourcesMap.find(Name) == ResourcesMap.end();
		}

		bool Add(SmartPointer<T>&& Resource, const std::string& Name, bool Replace = false)
		{
			bool NameFree = IsNameFree(Name);

			if (NameFree || Replace)
			{
				if (NameFree)
				{
					Resources.emplace_back(std::move(Resource));
					ResourcesMap[Name] = Resources.size() - 1;
				} else
				{
					Resources[ResourcesMap[Name]] = (SmartPointer<T>&&)std::move(Resource);
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

				auto Pred = [&](const SmartPointer<T>& A){ return A.Get() == Ptr; };
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

		T* Find(const std::string& Index)
		{
			auto It = ResourcesMap.find(Index);
			return It != ResourcesMap.end() ? Resources[It->second].Get() : nullptr;
		}

		SmartPointer<T>& operator[](size_t Index)
		{
			return Resources[Index];
		}

		SmartPointer<T>& operator[](const std::string& Index)
		{
			return Resources[ResourcesMap[Index]];
		}
	};

}


