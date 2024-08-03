#pragma once

#include <Graphics/Material.h>
#include <Scene/Scene.h>

namespace Columbus::Editor
{

	class ResourcesViewerMaterial
	{
	private:
		static bool Opened;
		static Material** Destination;
		static Material* Tmp;
	public:
		static void Open(Material** Dst)
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

		static void Draw(Scene& Scn);
	};

}


