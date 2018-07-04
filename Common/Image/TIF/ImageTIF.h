#include <Common/Image/Image.h>

namespace Columbus
{

	class ImageLoaderTIF : public ImageLoader
	{
	public:
		ImageLoaderTIF() {}

		static bool IsTIF(std::string FileName);

		virtual bool Load(std::string FileName);
		virtual void Free();

		virtual ~ImageLoaderTIF() override {}
	};

}














