#include <Graphics/Shader.h>
#include <Graphics/Device.h>
#include <System/Log.h>

namespace Columbus
{

	DefaultShaders::DefaultShaders()
	{
		Skybox = gDevice->CreateShaderProgram();
		SkyboxCubemapGeneration = gDevice->CreateShaderProgram();
		IrradianceGeneration = gDevice->CreateShaderProgram();
		PrefilterGeneration = gDevice->CreateShaderProgram();
		IntegrationGeneration = gDevice->CreateShaderProgram();

		Log::Initialization("Default shaders loading");

		Skybox->Load(ShaderProgram::StandartProgram::Skybox);
		SkyboxCubemapGeneration->Load(ShaderProgram::StandartProgram::SkyboxCubemapGeneration);
		IrradianceGeneration->Load(ShaderProgram::StandartProgram::IrradianceGeneration);
		PrefilterGeneration->Load(ShaderProgram::StandartProgram::PrefilterGeneration);
		IntegrationGeneration->Load(ShaderProgram::StandartProgram::IntegrationGeneration);

		Log::Initialization("Default shaders compiling");

		Skybox->Compile();
		SkyboxCubemapGeneration->Compile();
		IrradianceGeneration->Compile();
		PrefilterGeneration->Compile();
		IntegrationGeneration->Compile();
	}

	DefaultShaders::~DefaultShaders()
	{
		delete Skybox;
		delete SkyboxCubemapGeneration;
		delete IrradianceGeneration;
		delete PrefilterGeneration;
		delete IntegrationGeneration;
	}

}


