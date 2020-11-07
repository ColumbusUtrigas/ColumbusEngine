#pragma once

#include <Scene/Scene.h>
#include <Graphics/Mesh.h>

namespace Columbus::Editor
{

	class ResourcesViewerMesh
	{
	private:
		static bool Opened;
		static Mesh** Destination;
		static Mesh* Tmp;
	public:
		static void Open(Mesh** Dst)
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


