#pragma once

#include "Core/SmartPointer.h"
#include "Core/Types.h"
#include <cstddef>
#include <string_view>

namespace Columbus
{
	class DeviceVulkan;
	class Texture2;

	namespace ShaderDebugText
	{
		// Mirrored in Shaders/ShaderDebugText.hlsli. Packed ASCII, four bytes per word.
		struct alignas(16) String
		{
			u32 Characters[16]{};
			u32 Length = 0;
			u32 Padding[3]{};
		};

		static_assert(sizeof(String) == 80);
		static_assert(offsetof(String, Length) == 64);

		String MakeString(std::string_view Text);

		Texture2* CreateFontAtlas(SPtr<DeviceVulkan> Device);
		void DestroyFontAtlas(SPtr<DeviceVulkan> Device, Texture2*& Atlas);
	}
}
