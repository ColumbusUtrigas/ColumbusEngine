#include <Common/Image/Image.h>

namespace Columbus
{

	class ImageLoaderJPG : public ImageLoader
	{
	public:
		ImageLoaderJPG() {}

		static bool IsJPG(const char* FileName);

		virtual bool Load(const char* FileName);

		virtual ~ImageLoaderJPG() override {}
	};

}




















