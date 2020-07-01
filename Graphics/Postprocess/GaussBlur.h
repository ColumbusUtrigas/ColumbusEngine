#pragma once

#include <Graphics/Device.h>

namespace Columbus
{
	Texture* GaussBlur(Texture* base, iVector2 ContextSize, int mips);
}
