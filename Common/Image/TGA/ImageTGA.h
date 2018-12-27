#include <Common/Image/Image.h>

namespace Columbus
{

	class ImageLoaderTGA : public ImageLoader
	{
	public:
		ImageLoaderTGA() {}

		static bool IsTGA(std::string FileName);

		virtual bool Load(std::string FileName) override;

		virtual ~ImageLoaderTGA() {}
	};

}

















