#pragma once

#include <Common/Model/Model.h>

namespace Columbus
{

	class ModelLoaderCMF : public ModelLoader
	{
	public:
		ModelLoaderCMF();
		virtual bool Load(const std::string& FileName) final override;

		static bool IsCMF(const std::string& FileName);

		~ModelLoaderCMF();
	};

}


