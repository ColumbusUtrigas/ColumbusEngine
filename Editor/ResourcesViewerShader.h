#pragma once

#include <Scene/Scene.h>
#include <Graphics/Shader.h>

namespace Columbus::Editor
{

	class ResourcesViewerShader
	{
	private:
		static bool Opened;
		static ShaderProgram* Tmp;
		static Material* Mat;
	public:
		static void Open(Material* Dst)
		{
			Opened = true;
			Mat = Dst;
			if (Dst != nullptr)
			{
				Tmp = Dst->GetShader();
			}
		}

		static void Close()
		{
			Opened = false;
			if (Mat != nullptr)
			{
				Mat->SetShader(Tmp);
			}
		}

		static void Draw(Scene* Scn);
	};
}


