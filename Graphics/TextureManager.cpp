#include <Graphics/TextureManager.h>

namespace C
{

  void C_TextureManager::add(C_Texture* aTexture)
  {
    mTextures.push_back(aTexture);

    if (aTexture != nullptr)
      mSize += aTexture->getSize();
  }

  void C_TextureManager::setConfig(C_TextureConfig aConfig)
  {
    for (auto i = mTextures.begin(); i != mTextures.end(); i++)
    {
      C_Texture* a = *i;
      a->setConfig(aConfig);
    }
  }

  void C_TextureManager::reloadAll()
  {
    for (auto i = mTextures.begin(); i != mTextures.end(); i++)
    {
      C_Texture* a = *i;
      a->reload();
    }
  }

}
