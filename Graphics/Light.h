#pragma once

#include <Core/Reflection.h>
#include <Core/Types.h>

namespace Columbus
{

	enum class LightType : u32
	{
		Directional	= 0,
		Point		= 1,
		Spot		= 2,
		Rectangle	= 3,
		Disc        = 4,
		Line        = 5,

		Count       = 6
	};

	enum class ELightFlags : u32
	{
		TwoSided = 1 << 0,
		Shadow   = 1 << 1,
	};
	IMPLEMENT_ENUM_CLASS_BITOPS(ELightFlags);

	static const char* LightTypeToString(LightType type)
	{
		switch (type)
		{
		case LightType::Directional: return "Directional";
		case LightType::Point:       return "Point";
		case LightType::Spot:        return "Spot";
		case LightType::Rectangle:   return "Rectangle";
		default: return "Unkown";
		}
	}

}

CREFLECT_DECLARE_ENUM(Columbus::LightType, "CA1472F6-750C-4647-9FAC-B49252236033");
