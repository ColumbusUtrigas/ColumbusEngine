/************************************************
*              TextureManager.cpp               *
*************************************************
*          This file is a part of:              *
*               COLUMBUS ENGINE                 *
*************************************************
*             Nikolay(Columbus) Red             *
*                   20.07.2017                  *
*************************************************/

#include <Graphics/TextureManager.h>

namespace C
{

	//////////////////////////////////////////////////////////////////////////////
	//Constructor
	C_TextureManager::C_TextureManager() :
		mSmooth(true),
		mMipmaps(true),
		mAnisotropy(8)
	{

	}
	//////////////////////////////////////////////////////////////////////////////
	//Add texture
	void C_TextureManager::add(C_Texture* aTexture)
	{
		mTextures.push_back(aTexture);

		if (aTexture != nullptr)
			mSize += aTexture->getSize();
	}
	//////////////////////////////////////////////////////////////////////////////
	//Set texture config
	void C_TextureManager::setConfig(C_TextureConfig aConfig)
	{
		for (auto i : mTextures)
			i->setConfig(aConfig);
	}
	///////////////////////////////////////////////////////////////////////
	//Get images size
	size_t C_TextureManager::size()
	{
		return mSize;
	}
	///////////////////////////////////////////////////////////////////////
	//Destructor
	C_TextureManager::~C_TextureManager()
	{

	}

}
