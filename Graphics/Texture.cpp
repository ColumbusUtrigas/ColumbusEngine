#include <Graphics/Texture.h>
#include <Graphics/Device.h>
#include <System/Log.h>

#include <Graphics/PostEffect.h>
#include <Graphics/ScreenQuad.h>
#include <Graphics/OpenGL/ShaderOpenGL.h>

namespace Columbus
{

	static void CreateIntegrationMap(Texture*& IntegrationMap);

	DefaultTextures::DefaultTextures()
	{
		uint32 Zero = 0xFF000000;
		uint32 One  = 0xFFFFFFFF;

		Black = gDevice->CreateTexture();
		White = gDevice->CreateTexture();

		Log::Initialization("Default textures loading");

		TextureDesc Desc(1, 1, 0, 0, TextureFormat::RGBA8);

		Black->Create2D(Desc);
		White->Create2D(Desc);

		Black->Load(&Zero, Desc);
		White->Load(&One,  Desc);

		CreateIntegrationMap(IntegrationLUT);

		// TODO: EDITOR MODE
		if (true)
		{
			IconSun = gDevice->CreateTexture();
			IconLamp = gDevice->CreateTexture();
			IconAudio = gDevice->CreateTexture();
			IconFlashlight = gDevice->CreateTexture();
			IconParticles = gDevice->CreateTexture();

			IconSun->Load("Data/Icons/Sun.png");
			IconLamp->Load("Data/Icons/Lamp.png");
			IconFlashlight->Load("Data/Icons/Flashlight.png");
			IconAudio->Load("Data/Icons/Audio.png");
			IconParticles->Load("Data/Icons/Particles.png");
		}
	}

	DefaultTextures::~DefaultTextures()
	{
		delete Black;
		delete White;
		delete IntegrationLUT;

		delete IconSun;
		delete IconLamp;
		delete IconFlashlight;
		delete IconAudio;
		delete IconParticles;
	}

	void CreateIntegrationMap(Texture*& IntegrationMap)
	{
		auto IntegrationShader = gDevice->GetDefaultShaders()->IntegrationGeneration;
		uint32 Resolution = 256;
		TextureFormat Format = TextureFormat::RG16F;

		if (IntegrationMap == nullptr)
		{
			Texture::Flags Flags;
			Flags.AnisotropyFilter = Texture::Anisotropy::Anisotropy1;
			Flags.Filtering = Texture::Filter::Linear;
			Flags.Wrapping = Texture::Wrap::Clamp;

			IntegrationMap = gDevice->CreateTexture();
			IntegrationMap->Create2D(TextureDesc(Resolution, Resolution, 0, 0, Format));
			IntegrationMap->SetFlags(Flags);
		}

		PostEffect Frame;
		ScreenQuad Quad;
		Frame.ColorTexturesEnablement[0] = true;
		Frame.ColorTexturesFormats[0] = Format;

		Frame.Bind({ 0 }, { 0, 0 }, { 1, 1 });

		auto Tmp = Frame.ColorTextures[0];
		Frame.ColorTextures[0] = IntegrationMap;

		Frame.Bind({ 0 }, { 0, 0}, { (int)Resolution, (int)Resolution });
		((ShaderProgramOpenGL*)IntegrationShader)->Bind();
		Quad.Render();
		((ShaderProgramOpenGL*)IntegrationShader)->Unbind();
		Frame.Mipmaps();
		Frame.Unbind();

		Frame.ColorTextures[0] = Tmp;
	}

}


