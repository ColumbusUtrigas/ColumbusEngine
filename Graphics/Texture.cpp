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
		uint32 Zero = 0x000000FF;
		uint32 One  = 0xFFFFFFFF;

		Black = gDevice->CreateTexture();
		White = gDevice->CreateTexture();

		Log::Initialization("Default textures loading");

		Texture::Properties Props = Texture::Properties(1, 1, 0, TextureFormat::RGBA8);

		Black->Create2D(Props);
		White->Create2D(Props);

		Black->Load(&Zero, Props);
		White->Load(&One,  Props);

		CreateIntegrationMap(IntegrationLUT);
	}

	DefaultTextures::~DefaultTextures()
	{
		delete Black;
		delete White;
		delete IntegrationLUT;
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
			Flags.Wrapping = Texture::Wrap::ClampToEdge;

			IntegrationMap = gDevice->CreateTexture();
			IntegrationMap->Create2D(Texture::Properties{ Resolution, Resolution, 0, Format });
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
		Frame.Unbind();

		Frame.ColorTextures[0] = Tmp;
	}

}


