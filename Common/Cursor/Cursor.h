#pragma once

#include <Common/Image/Image.h>
#include <cstring>

namespace Columbus
{

	struct CursorFrame
	{
		uint8 Width = 0;
		uint8 Height = 0;
		uint16 HotPointX = 0;
		uint16 HotPointY = 0;
		uint8* PixelData = nullptr;
	};

	struct Cursor
	{
		uint16 FramesCount = 0;
		CursorFrame* Frames = nullptr;

		Cursor(Image& InImage, uint16 InHotPointX, uint16 InHotPointY)
		{
			FramesCount = 1;
			Frames = new CursorFrame[1];
			Frames[0].Width = InImage.GetWidth();
			Frames[0].Height = InImage.GetHeight();
			Frames[0].HotPointX = InHotPointX;
			Frames[0].HotPointY = InHotPointY;

			uint64 Size = InImage.GetWidth() * InImage.GetHeight() * InImage.GetBytesPerPixel();

			Frames[0].PixelData = new uint8[(size_t)Size];
			memcpy(Frames[0].PixelData, InImage.GetData(), (size_t)Size);
		}
	};

}


