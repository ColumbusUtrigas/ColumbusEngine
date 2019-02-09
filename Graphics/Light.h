#pragma once

#include <Math/Vector3.h>

namespace Columbus
{

	class Light
	{
	public:
		Vector3 Color = Vector3(1, 1, 1);
		Vector3 Pos = Vector3(0, 0, 0);
		Vector3 Dir = Vector3(-0.5f, -0.4f, -0.3f);

		int Type = 0;

		float Range = 10.0f;
		float InnerCutoff = 12.5f;
		float OuterCutoff = 17.5f;
	public:
		Light();
		Light(int Type, const Vector3& Pos = Vector3(0, 0, 0));
		Light(const char* FileName, const Vector3& Pos = Vector3(0, 0, 0));

		bool Load(const char* FileName);
		
		~Light();
	};

}


