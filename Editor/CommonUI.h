#include <imgui/imgui.h>

namespace Columbus
{

	struct CommonUISettings
	{
		static float TooltipDelay;
	};

	void FlagButton(const char* name, bool& enabled);
	void ShowTooltipDelayed(float delay, const char* fmt, ...);

}
