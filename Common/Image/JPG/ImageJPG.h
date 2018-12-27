#include <Common/Image/Image.h>

namespace Columbus
{

	class ImageLoaderJPG : public ImageLoader
	{
	public:
		ImageLoaderJPG() {}

		static bool IsJPG(std::string FileName);

		virtual bool Load(std::string FileName);

		virtual ~ImageLoaderJPG() override {}
	};

}




















