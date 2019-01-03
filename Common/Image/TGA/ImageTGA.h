#include <Common/Image/Image.h>

namespace Columbus
{

	class ImageLoaderTGA : public ImageLoader
	{
	public:
		ImageLoaderTGA() {}

		static bool IsTGA(const char* FileName);

		virtual bool Load(const char* FileName) override;

		virtual ~ImageLoaderTGA() {}
	};

}

















