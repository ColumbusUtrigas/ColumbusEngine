#pragma once

#include <Common/Image/Image.h>

namespace Columbus
{

	class ImageLoaderHDR : public ImageLoader
	{
	public:
		ImageLoaderHDR() {}

		static bool IsHDR(const char* FileName);
		
		virtual bool Load(const char* FileName) final override;

		virtual ~ImageLoaderHDR() final override {}
	};

}


