#pragma once

#include <Common/Model/Model.h>

namespace Columbus
{

	class ModelLoaderCMF : public ModelLoader
	{
	public:
		ModelLoaderCMF();
		virtual bool Load(const char* FileName) final override;

		static bool IsCMF(const char* FileName);

		~ModelLoaderCMF();
	};

}


