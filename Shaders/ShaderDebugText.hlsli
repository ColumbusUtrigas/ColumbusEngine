#pragma once

// Set/binding are selectable by each consumer; match the font atlas binding on the CPU.
#ifndef SHADER_DEBUG_TEXT_SET
#define SHADER_DEBUG_TEXT_SET 1
#endif

#ifndef SHADER_DEBUG_TEXT_BINDING
#define SHADER_DEBUG_TEXT_BINDING 0
#endif

// Atlas layout mirrors Graphics/Core/ShaderDebugText.cpp: ASCII 32..126 in 8x12 cells.
[[vk::binding(SHADER_DEBUG_TEXT_BINDING, SHADER_DEBUG_TEXT_SET)]] Texture2D<float> ShaderDebugFontAtlas;

static const uint2 DebugTextGlyphSize = uint2(8, 12);
static const uint  DebugTextAtlasColumns = 16;

// Mirrors ShaderDebugText::String. Can be supplied through constants or a structured buffer.
struct ShaderDebugString
{
	uint4 Characters[4];
	uint  Length;
	uint  Padding0;
	uint  Padding1;
	uint  Padding2;
};

ShaderDebugString DebugTextEmpty()
{
	return (ShaderDebugString)0;
}

uint DebugTextCharacter(ShaderDebugString Text, uint Index)
{
	const uint Word = Text.Characters[Index / 16][(Index / 4) % 4];
	return (Word >> ((Index % 4) * 8)) & 255;
}

void DebugTextAppendCharacter(inout ShaderDebugString Text, uint Character)
{
	if (Text.Length < 64)
	{
		const uint Index = Text.Length++;
		Text.Characters[Index / 16][(Index / 4) % 4] |= (Character & 255) << ((Index % 4) * 8);
	}
}

// Append whole strings from ShaderDebugText::MakeString, or compose text in the shader.
void DebugTextAppendString(inout ShaderDebugString Text, ShaderDebugString Suffix)
{
	for (uint i = 0; i < min(Suffix.Length, 64u) && Text.Length < 64; ++i)
	{
		DebugTextAppendCharacter(Text, DebugTextCharacter(Suffix, i));
	}
}

void DebugTextAppendUInt(inout ShaderDebugString Text, uint Value)
{
	uint Divisor = 1;

	while (Value / Divisor >= 10)
	{
		Divisor *= 10;
	}

	do
	{
		DebugTextAppendCharacter(Text, '0' + Value / Divisor);
		Value %= Divisor;
		Divisor /= 10;
	}
	while (Divisor != 0);
}

float DebugTextGlyph(uint Character, float2 LocalPixel)
{
	if (any(LocalPixel < 0) || any(LocalPixel >= DebugTextGlyphSize))
	{
		return 0;
	}

	if (Character < 32 || Character > 126)
	{
		Character = '?';
	}

	const uint Glyph = Character - 32;
	const uint2 AtlasPixel = uint2(Glyph % DebugTextAtlasColumns, Glyph / DebugTextAtlasColumns) * DebugTextGlyphSize + uint2(LocalPixel);
	return ShaderDebugFontAtlas.Load(int3(AtlasPixel, 0));
}

// Advance is in font pixels: use (8,12) for general ASCII, (5,12) for compact numbers.
// Pixel/Origin/Scale are in the consumer's output pixel coordinates. Newline and tab are supported.
float DebugTextDrawString(ShaderDebugString Text, float2 Pixel, float2 Origin, float2 Scale, float2 Advance)
{
	const float2 LocalPixel = (Pixel - Origin) / max(Scale, 0.001);
	float2 Cursor = 0;
	float Coverage = 0;

	for (uint i = 0; i < min(Text.Length, 64u); ++i)
	{
		const uint Character = DebugTextCharacter(Text, i);

		if (Character == '\n')
		{
			Cursor.x = 0;
			Cursor.y += Advance.y;
		}
		else if (Character == '\t')
		{
			Cursor.x += 4 * Advance.x;
		}
		else
		{
			Coverage = max(Coverage, DebugTextGlyph(Character, LocalPixel - Cursor));
			Cursor.x += Advance.x;
		}
	}

	return Coverage;
}
