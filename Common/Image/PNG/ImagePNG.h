#include <Common/Image/Image.h>

namespace Columbus
{

	class ImageLoaderPNG : public ImageLoader
	{
	public:
		ImageLoaderPNG() {}

		static bool IsPNG(const char* FileName);

		virtual bool Load(const char* FileName) override;

		virtual ~ImageLoaderPNG() override {}
	};

}


