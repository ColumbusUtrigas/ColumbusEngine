#include <Common/Image/Image.h>

namespace Columbus
{

	class ImageLoaderBMP : public ImageLoader
	{
	public:
		ImageLoaderBMP() {}

		static bool IsBMP(std::string FileName);

		virtual bool Load(std::string FileName) override;
		virtual void Free() override;

		virtual ~ImageLoaderBMP() override {}
	};

}

















