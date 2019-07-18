#include <Graphics/Shader.h>
#include <Graphics/Device.h>
#include <System/Log.h>

namespace Columbus
{

	DefaultShaders::DefaultShaders()
	{
		ScreenSpace = gDevice->CreateShaderProgram();
		Tonemap = gDevice->CreateShaderProgram();
		ResolveMSAA = gDevice->CreateShaderProgram();
		GaussBlur = gDevice->CreateShaderProgram();
		BloomBright = gDevice->CreateShaderProgram();
		Bloom = gDevice->CreateShaderProgram();
		Vignette = gDevice->CreateShaderProgram();
		FXAA = gDevice->CreateShaderProgram();
		Icon = gDevice->CreateShaderProgram();
		EditorTools = gDevice->CreateShaderProgram();
		Skybox = gDevice->CreateShaderProgram();
		SkyboxCubemapGeneration = gDevice->CreateShaderProgram();
		IrradianceGeneration = gDevice->CreateShaderProgram();
		PrefilterGeneration = gDevice->CreateShaderProgram();
		IntegrationGeneration = gDevice->CreateShaderProgram();

		printf("\n");
		Log::Initialization("Default shaders loading");

		ScreenSpace->Load(ShaderProgram::StandartProgram::ScreenSpace);
		Tonemap->Load(ShaderProgram::StandartProgram::Tonemap);
		ResolveMSAA->Load(ShaderProgram::StandartProgram::ResolveMSAA);
		GaussBlur->Load(ShaderProgram::StandartProgram::GaussBlur);
		BloomBright->Load(ShaderProgram::StandartProgram::BloomBright);
		Bloom->Load(ShaderProgram::StandartProgram::Bloom);
		Vignette->Load(ShaderProgram::StandartProgram::Vignette);
		FXAA->Load(ShaderProgram::StandartProgram::FXAA);
		Icon->Load(ShaderProgram::StandartProgram::Icon);
		EditorTools->Load(ShaderProgram::StandartProgram::EditorTools);
		Skybox->Load(ShaderProgram::StandartProgram::Skybox);
		SkyboxCubemapGeneration->Load(ShaderProgram::StandartProgram::SkyboxCubemapGeneration);
		IrradianceGeneration->Load(ShaderProgram::StandartProgram::IrradianceGeneration);
		PrefilterGeneration->Load(ShaderProgram::StandartProgram::PrefilterGeneration);
		IntegrationGeneration->Load(ShaderProgram::StandartProgram::IntegrationGeneration);

		printf("\n");
		Log::Initialization("Default shaders compiling");

		ScreenSpace->Compile();
		Tonemap->Compile();
		ResolveMSAA->Compile();
		GaussBlur->Compile();
		BloomBright->Compile();
		Bloom->Compile();
		Vignette->Compile();
		FXAA->Compile();
		Icon->Compile();
		EditorTools->Compile();
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
		delete ResolveMSAA;
		delete GaussBlur;
		delete BloomBright;
		delete Bloom;
		delete Vignette;
		delete FXAA;
		delete Icon;
		delete EditorTools;
		delete Skybox;
		delete SkyboxCubemapGeneration;
		delete IrradianceGeneration;
		delete PrefilterGeneration;
		delete IntegrationGeneration;
	}

}


