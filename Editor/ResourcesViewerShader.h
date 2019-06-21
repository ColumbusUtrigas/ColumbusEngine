#pragma once

#include <Scene/Scene.h>
#include <Graphics/Shader.h>

namespace Columbus
{

	class ResourcesViewerShader
	{
	private:
		static bool Opened;
		static ShaderProgram** Destination;
		static ShaderProgram* Tmp;
	public:
		static void Open(ShaderProgram** Dst)
		{
			Opened = true;
			Destination = Dst;
			if (Dst != nullptr)
			{
				Tmp = *Dst;
			}
		}

		static void Close()
		{
			Opened = false;
			if (Destination != nullptr)
			{
				*Destination = Tmp;
			}
		}

		static void Draw(Scene* Scn);
	};
}


