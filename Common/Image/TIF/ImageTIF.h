#include <Common/Image/Image.h>

namespace Columbus
{

	class ImageLoaderTIF : public ImageLoader
	{
	public:
		ImageLoaderTIF() {}

		static bool IsTIF(const char* FileName);

		virtual bool Load(const char* FileName) override;

		virtual ~ImageLoaderTIF() override {}
	};

}


