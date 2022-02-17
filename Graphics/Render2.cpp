#include <Graphics/Render2.h>
#include <Graphics/Device.h>

namespace Columbus
{

	void Render2::Render()
	{
		gDevice->BeginMarker("Render");

		gDevice->EndMarker();
	}

}
