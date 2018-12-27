#include <Common/Image/Image.h>

namespace Columbus
{

	class ImageLoaderPNG : public ImageLoader
	{
	public:
		ImageLoaderPNG() {}

		static bool IsPNG(std::string FileName);

		virtual bool Load(std::string FileName) override;

		virtual ~ImageLoaderPNG() override {}
	};

}
























