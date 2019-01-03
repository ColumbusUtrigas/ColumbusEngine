#pragma once

#include <Math/Vector3.h>

namespace Columbus
{

	class Light
	{
	public:
		int Type = 0;

		Vector3 Pos = Vector3(0, 0, 0);
		Vector3 Dir = Vector3(-0.5f, -0.4f, -0.3f);

		Vector3 Color = Vector3(1, 1, 1);

		float Constant = 1.0f;
		float Linear = 0.09f;
		float Quadratic = 0.032f;

		float InnerCutoff = 12.5f;
		float OuterCutoff = 17.5f;
	public:
		Light();
		Light(int Type, const Vector3& Pos = Vector3(0, 0, 0));
		Light(const char* File, const Vector3& Pos = Vector3(0, 0, 0));

		bool SaveToXML(const char* File) const;
		bool LoadFromXML(const char* File);

		bool Load(const char* File);
		
		~Light();
	};

}
