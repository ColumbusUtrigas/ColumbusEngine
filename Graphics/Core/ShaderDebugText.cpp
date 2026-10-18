#include "ShaderDebugText.h"
#include "Core/Assert.h"
#include "Graphics/Vulkan/DeviceVulkan.h"
#include "Profiling/Profiling.h"
#include "stb_easy_font.h"

#include <algorithm>
#include <array>

namespace Columbus::ShaderDebugText
{
	DECLARE_MEMORY_PROFILING_COUNTER(MemoryCounterShaderDebugFont);
	IMPLEMENT_MEMORY_PROFILING_COUNTER("Shader debug font", "RenderMemory", MemoryCounterShaderDebugFont);

	static constexpr u32 GlyphWidth   = 8;
	static constexpr u32 GlyphHeight  = 12;
	static constexpr u32 AtlasColumns = 16;
	static constexpr u32 AtlasRows    = 6;
	static constexpr u32 AtlasWidth   = GlyphWidth * AtlasColumns;
	static constexpr u32 AtlasHeight  = GlyphHeight * AtlasRows;

	String MakeString(std::string_view Text)
	{
		String Result;
		COLUMBUS_ASSERT_MESSAGE(Text.size() <= 64, "Shader debug strings support up to 64 ASCII characters");
		Result.Length = u32(std::min(Text.size(), size_t(64)));

		for (u32 i = 0; i < Result.Length; ++i)
		{
			const u32 Character = u8(Text[i]);
			Result.Characters[i / 4] |= Character << ((i % 4) * 8);
		}

		return Result;
	}

	static void RasterizeGlyph(std::array<u8, AtlasWidth * AtlasHeight>& Pixels, u32 Character)
	{
		struct FontVertex
		{
			float X, Y, Z;
			u8    Colour[4];
		};
		static_assert(sizeof(FontVertex) == 16);

		std::array<FontVertex, 256> Vertices{};
		char Text[2] = { char(Character), 0 };
		const int QuadCount = stb_easy_font_print(0, 0, Text, nullptr, Vertices.data(), int(sizeof(Vertices)));
		const u32 GlyphIndex = Character - 32;
		const u32 OriginX    = (GlyphIndex % AtlasColumns) * GlyphWidth;
		const u32 OriginY    = (GlyphIndex / AtlasColumns) * GlyphHeight;

		for (int i = 0; i < QuadCount; ++i)
		{
			const auto& Min = Vertices[i * 4];
			const auto& Max = Vertices[i * 4 + 2];
			COLUMBUS_ASSERT(Min.X >= 0 && Min.Y >= 0 && Max.X <= GlyphWidth && Max.Y <= GlyphHeight);

			for (u32 y = u32(Min.Y); y < std::min(u32(Max.Y), GlyphHeight); ++y)
			{
				for (u32 x = u32(Min.X); x < std::min(u32(Max.X), GlyphWidth); ++x)
				{
					Pixels[(OriginY + y) * AtlasWidth + OriginX + x] = 255;
				}
			}
		}
	}

	Texture2* CreateFontAtlas(SPtr<DeviceVulkan> Device)
	{
		std::array<u8, AtlasWidth * AtlasHeight> Pixels{};

		for (u32 i = 32; i <= 126; ++i)
		{
			RasterizeGlyph(Pixels, i);
		}

		TextureDesc2 Desc;
		Desc.Width     = AtlasWidth;
		Desc.Height    = AtlasHeight;
		Desc.Format    = TextureFormat::R8;
		Desc.Usage     = TextureUsage::Sampled;

		Texture2* Atlas = Device->CreateTexture(Desc);
		Device->SetDebugName(Atlas, "ShaderDebugFontAtlas");
		Device->UploadTextureMipData(Atlas, 0, 0, Pixels.data());
		AddProfilingMemory(MemoryCounterShaderDebugFont, Atlas->GetSize());
		return Atlas;
	}

	void DestroyFontAtlas(SPtr<DeviceVulkan> Device, Texture2*& Atlas)
	{
		if (Atlas)
		{
			RemoveProfilingMemory(MemoryCounterShaderDebugFont, Atlas->GetSize());
			Device->DestroyTexture(Atlas);
			Atlas = nullptr;
		}
	}
}
