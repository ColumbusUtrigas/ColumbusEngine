#pragma once

#include <Common/Image/Image.h>

namespace Columbus
{

	class ImageLoaderEXR : public ImageLoader
	{
	public:
		ImageLoaderEXR() {}

		static bool IsEXR(const char* Filename);

		virtual bool Load(const char* Filename) final override;

		virtual ~ImageLoaderEXR() {}
	};

}


