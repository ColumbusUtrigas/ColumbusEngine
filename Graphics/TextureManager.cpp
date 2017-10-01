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
    for (auto i = mTextures.begin(); i != mTextures.end(); i++)
    {
      C_Texture* a = *i;
      a->setConfig(aConfig);
    }
  }
  //////////////////////////////////////////////////////////////////////////////
  //Reload all textures
  void C_TextureManager::reloadAll()
  {
    for (auto i = mTextures.begin(); i != mTextures.end(); i++)
    {
      C_Texture* a = *i;
      a->reload();
    }
  }
  //////////////////////////////////////////////////////////////////////////////
  //Get images size
  size_t C_TextureManager::size()
  {
    return mSize;
  }

}
