#include <Graphics/Shader.h>
#include <Graphics/Device.h>
#include <System/Log.h>

namespace Columbus
{

	DefaultShaders::DefaultShaders()
	{
		Final = gDevice->CreateShaderProgram();
		GaussBlur = gDevice->CreateShaderProgram();
		BloomBright = gDevice->CreateShaderProgram();
		Bloom = gDevice->CreateShaderProgram();
		Icon = gDevice->CreateShaderProgram();
		Skybox = gDevice->CreateShaderProgram();
		SkyboxCubemapGeneration = gDevice->CreateShaderProgram();
		IrradianceGeneration = gDevice->CreateShaderProgram();
		PrefilterGeneration = gDevice->CreateShaderProgram();
		IntegrationGeneration = gDevice->CreateShaderProgram();

		printf("\n");
		Log::Initialization("Default shaders loading");

		Final->Load(ShaderProgram::StandartProgram::Final);
		GaussBlur->Load(ShaderProgram::StandartProgram::GaussBlur);
		BloomBright->Load(ShaderProgram::StandartProgram::BloomBright);
		Bloom->Load(ShaderProgram::StandartProgram::Bloom);
		Icon->Load(ShaderProgram::StandartProgram::Icon);
		Skybox->Load(ShaderProgram::StandartProgram::Skybox);
		SkyboxCubemapGeneration->Load(ShaderProgram::StandartProgram::SkyboxCubemapGeneration);
		IrradianceGeneration->Load(ShaderProgram::StandartProgram::IrradianceGeneration);
		PrefilterGeneration->Load(ShaderProgram::StandartProgram::PrefilterGeneration);
		IntegrationGeneration->Load(ShaderProgram::StandartProgram::IntegrationGeneration);

		printf("\n");
		Log::Initialization("Default shaders compiling");

		Final->Compile();
		GaussBlur->Compile();
		BloomBright->Compile();
		Bloom->Compile();
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
		delete Final;
		delete GaussBlur;
		delete BloomBright;
		delete Bloom;
		delete Icon;
		delete Skybox;
		delete SkyboxCubemapGeneration;
		delete IrradianceGeneration;
		delete PrefilterGeneration;
		delete IntegrationGeneration;
	}

}


