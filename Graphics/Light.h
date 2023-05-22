#pragma once

#include <Math/Vector3.h>
#include <Core/ISerializable.h>

namespace Columbus
{

	enum class LightType : uint8_t
	{
		Directional	= 0,
		Point		= 1,
		Spot		= 2,

		Rectangle	= 3,
		Sphere		= 4,
	};

	struct Light : public ISerializable
	{
		enum
		{
			Directional = 0,
			Point = 1,
			Spot = 2
		};

		Vector3 Color = Vector3(1, 1, 1);
		Vector3 Pos = Vector3(0, 0, 0);
		Vector3 Dir = Vector3(1, 0, 0);

		int Type = Directional;
		bool Shadows = false;

		float Energy = 1.0f;
		float Range = 10.0f;
		float InnerCutoff = 12.5f;
		float OuterCutoff = 17.5f;

		virtual void Serialize(JSON& J) const final override;
		virtual void Deserialize(JSON& J) final override;

		virtual ~Light() {}
	};

}


