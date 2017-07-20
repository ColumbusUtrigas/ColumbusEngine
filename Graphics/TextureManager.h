/************************************************
*               TextureManager.h                *
*************************************************
*          This file is a part of:              *
*               COLUMBUS ENGINE                 *
*************************************************
*             Nikolay(Columbus) Red             *
*                   20.07.2017                  *
*************************************************/

#pragma once

#include <list>

#include <Graphics/Texture.h>

namespace C
{

  class C_TextureManager
  {
  private:
    bool mSmooth = true;
    bool mMipmaps = true;
    unsigned int mAnisotropy = 8;

    std::list<C_Texture*> mTextures;

    size_t mSize = 0;
  public:
    //Constructor
    inline C_TextureManager() {}
    //Add texture
    void add(C_Texture* aTexture);
    //Set texture config
    void setConfig(C_TextureConfig aConfig);
    //Reload all textures
    void reloadAll();
    //Destructor
    inline ~C_TextureManager() {}
  };

}
