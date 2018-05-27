/************************************************
*              TextureManager.cpp               *
*************************************************
*          This file is a part of:              *
*               COLUMBUS ENGINE                 *
*************************************************
*                Nika(Columbus) Red             *
*                   20.07.2017                  *
*************************************************/

#include <Graphics/TextureManager.h>

namespace Columbus
{

	//////////////////////////////////////////////////////////////////////////////
	//Constructor
	TextureManager::TextureManager()
	{

	}
	//////////////////////////////////////////////////////////////////////////////
	//Add texture
	void TextureManager::add(Texture* aTexture)
	{
		mTextures.push_back(aTexture);

		if (aTexture != nullptr)
		{
			mSize += aTexture->GetSize();
		}
	}
	//////////////////////////////////////////////////////////////////////////////
	//Set texture config
	void TextureManager::setConfig(TextureConfig aConfig)
	{
		for (auto i : mTextures)
			i->setConfig(aConfig);
	}
	///////////////////////////////////////////////////////////////////////
	//Get images size
	size_t TextureManager::size()
	{
		return mSize;
	}
	///////////////////////////////////////////////////////////////////////
	//Destructor
	TextureManager::~TextureManager()
	{

	}

}
