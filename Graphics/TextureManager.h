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
    inline C_TextureManager() {}

    void add(C_Texture* aTexture);

    void setConfig(C_TextureConfig aConfig);

    void reloadAll();

    inline ~C_TextureManager() {}
  };

}
