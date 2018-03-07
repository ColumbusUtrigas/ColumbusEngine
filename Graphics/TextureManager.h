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
		bool mSmooth = true;
		bool mMipmaps = true;
		unsigned int mAnisotropy = 8;

		std::list<Texture*> mTextures;

		size_t mSize = 0;
	public:
		//Constructor
		TextureManager();
		//Add texture
		void add(Texture* aTexture);
		//Set texture config
		void setConfig(TextureConfig aConfig);
		//Get images size
		size_t size();
		//Destructor
		~TextureManager();
	};

}
