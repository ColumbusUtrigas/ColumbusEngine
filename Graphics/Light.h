#pragma once

#include <Math/Vector2.h>
#include <Math/Vector3.h>

namespace Columbus
{

	enum class LightType : u32
	{
		Directional	= 0,
		Point		= 1,
		Spot		= 2,
		Rectangle	= 3,

		Count       = 4
	};

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

	// TODO: make parameters more physical
	// TODO: support for IES profiles
	struct Light
	{
		Vector3 Color = Vector3(1, 1, 1);
		Vector3 Pos = Vector3(0, 0, 0);
		Vector3 Dir = Vector3(1, 0, 0);
		Vector2 Size = Vector2(1, 1);

		float Energy = 1.0f;
		float Range = 10.0f;
		float InnerCutoff = 12.5f;
		float OuterCutoff = 17.5f;

		LightType Type = LightType::Directional;
		bool Shadows = false;
	};

}
