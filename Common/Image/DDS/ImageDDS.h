#include <Common/Image/Image.h>

namespace Columbus
{

	class ImageLoaderDDS : public ImageLoader
	{
	public:
		ImageLoaderDDS() {}

		static bool IsDDS(const char* FileName);

		virtual bool Load(const char* FileName) override;

		virtual ~ImageLoaderDDS() override {}
	};

}












