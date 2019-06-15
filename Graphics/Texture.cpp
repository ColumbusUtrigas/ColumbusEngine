#include <Graphics/Texture.h>
#include <Graphics/Device.h>
#include <System/Log.h>

namespace Columbus
{

	DefaultTextures::DefaultTextures()
	{
		uint32 Zero = 0x00000000;
		uint32 One  = 0xFFFFFFFF;

		Black = gDevice->CreateTexture();
		White = gDevice->CreateTexture();

		Log::Initialization("Default textures loading");

		Texture::Properties Props = Texture::Properties(1, 1, 0, TextureFormat::RGBA8);

		Black->Create2D(Props);
		White->Create2D(Props);

		Black->Load(&Zero, Props);
		White->Load(&One,  Props);
	}

	DefaultTextures::~DefaultTextures()
	{
		delete Black;
		delete White;
	}

}


