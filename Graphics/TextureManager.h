/************************************************
*               TextureManager.h                *
*************************************************
*          This file is a part of:              *
*               COLUMBUS ENGINE                 *
*************************************************
*                Nika(Columbus) Red             *
*                   20.07.2017                  *
*************************************************/

#pragma once

#include <list>

#include <Graphics/Texture.h>

namespace Columbus
{

	class TextureManager
	{
	private:
		std::list<Texture*> mTextures;

		size_t mSize = 0;
	public:
		TextureManager();

		void add(Texture* aTexture);
		void setConfig(TextureConfig aConfig);
		size_t size();
		
		~TextureManager();
	};

}
