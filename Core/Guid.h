#pragma once

#include "Types.h"

namespace Columbus
{

	struct HGuid
	{
		HGuid();
		HGuid(u64 InGuid) : Guid(InGuid) {}
		HGuid(const HGuid& Other) : Guid(Other.Guid) {}
		HGuid(HGuid&& Other) noexcept : Guid(Other.Guid) { Other.Guid = 0; }
		HGuid& operator=(const HGuid& Other) { Guid = Other.Guid; return *this; }
		HGuid& operator=(HGuid&& Other) noexcept { Guid = Other.Guid; Other.Guid = 0; return *this; }

		bool operator==(const HGuid& Other) const { return Guid == Other.Guid; }
		bool operator!=(const HGuid& Other) const { return Guid != Other.Guid; }

		bool operator==(u64 Other) const { return Guid == Other; }
		bool operator!=(u64 Other) const { return Guid != Other; }

		operator u64() const { return Guid; }
		operator bool() const { return Guid != 0; }

		bool IsValid() const { return Guid != 0; }
	private:
		u64 Guid;
	};
}

namespace std
{
	// std::hash specialization for HGuid

	/*template <typename T> struct hash;

	template<>
	struct hash<uint64_t>;

	template<>
	struct hash<Columbus::HGuid>
	{
		size_t operator()(const Columbus::HGuid& Guid) const
		{
			return std::hash<uint64_t>()(Guid);
		}
	};*/
}
