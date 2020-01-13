#include <Common/Image/Image.h>

namespace Columbus
{

	class ImageLoaderBMP : public ImageLoader
	{
	public:
		ImageLoaderBMP() {}

		static bool IsBMP(const char* FileName);

		virtual bool Load(const char* FileName) override;

		virtual ~ImageLoaderBMP() override {}
	};

}
