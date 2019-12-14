#pragma once

#include <Common/Model/Model.h>

namespace Columbus
{

	class ModelLoaderOBJ : public ModelLoader
	{
	public:
		virtual bool  Load(const char* FileName) final override;
		static  bool IsOBJ(const char* FileName);
		virtual ~ModelLoaderOBJ() final override {}
	};

}
