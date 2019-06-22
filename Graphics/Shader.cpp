#include <Graphics/Shader.h>
#include <Graphics/Device.h>
#include <System/Log.h>

namespace Columbus
{

	DefaultShaders::DefaultShaders()
	{
		ScreenSpace = gDevice->CreateShaderProgram();
		Tonemap = gDevice->CreateShaderProgram();
		GaussBlur = gDevice->CreateShaderProgram();
		BloomBright = gDevice->CreateShaderProgram();
		Bloom = gDevice->CreateShaderProgram();
		FXAA = gDevice->CreateShaderProgram();
		Icon = gDevice->CreateShaderProgram();
		Skybox = gDevice->CreateShaderProgram();
		SkyboxCubemapGeneration = gDevice->CreateShaderProgram();
		IrradianceGeneration = gDevice->CreateShaderProgram();
		PrefilterGeneration = gDevice->CreateShaderProgram();
		IntegrationGeneration = gDevice->CreateShaderProgram();

		printf("\n");
		Log::Initialization("Default shaders loading");

		ScreenSpace->Load(ShaderProgram::StandartProgram::ScreenSpace);
		Tonemap->Load(ShaderProgram::StandartProgram::Tonemap);
		GaussBlur->Load(ShaderProgram::StandartProgram::GaussBlur);
		BloomBright->Load(ShaderProgram::StandartProgram::BloomBright);
		Bloom->Load(ShaderProgram::StandartProgram::Bloom);
		FXAA->Load(ShaderProgram::StandartProgram::FXAA);
		Icon->Load(ShaderProgram::StandartProgram::Icon);
		Skybox->Load(ShaderProgram::StandartProgram::Skybox);
		SkyboxCubemapGeneration->Load(ShaderProgram::StandartProgram::SkyboxCubemapGeneration);
		IrradianceGeneration->Load(ShaderProgram::StandartProgram::IrradianceGeneration);
		PrefilterGeneration->Load(ShaderProgram::StandartProgram::PrefilterGeneration);
		IntegrationGeneration->Load(ShaderProgram::StandartProgram::IntegrationGeneration);

		printf("\n");
		Log::Initialization("Default shaders compiling");

		ScreenSpace->Compile();
		Tonemap->Compile();
		GaussBlur->Compile();
		BloomBright->Compile();
		Bloom->Compile();
		FXAA->Compile();
		Icon->Compile();
		Skybox->Compile();
		SkyboxCubemapGeneration->Compile();
		IrradianceGeneration->Compile();
		PrefilterGeneration->Compile();
		IntegrationGeneration->Compile();

		printf("\n");
	}

	DefaultShaders::~DefaultShaders()
	{
		delete ScreenSpace;
		delete Tonemap;
		delete GaussBlur;
		delete BloomBright;
		delete Bloom;
		delete FXAA;
		delete Icon;
		delete Skybox;
		delete SkyboxCubemapGeneration;
		delete IrradianceGeneration;
		delete PrefilterGeneration;
		delete IntegrationGeneration;
	}

}


