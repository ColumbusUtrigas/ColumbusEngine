#include <Common/Image/Image.h>

namespace Columbus
{

	class ImageLoaderDDS : public ImageLoader
	{
	public:
		ImageLoaderDDS() {}

		static bool IsDDS(std::string FileName);

		virtual bool Load(std::string FileName) override;
		virtual void Free() override;

		virtual ~ImageLoaderDDS() override {}
	};

}












