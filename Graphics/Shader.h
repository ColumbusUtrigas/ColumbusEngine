#pragma once

namespace Columbus
{

	class ShaderProgram
	{
	protected:
		bool Loaded;
		bool Compiled;
		bool Error;
	public:
		enum class StandartProgram
		{
			Final,
			GaussBlur,
			BloomBright,
			Bloom,
			Icon,
			Skybox,
			SkyboxCubemapGeneration,
			IrradianceGeneration,
			PrefilterGeneration,
			IntegrationGeneration
		};
	public:
		ShaderProgram() {}

		bool IsLoaded()   const { return Loaded; }
		bool IsCompiled() const { return Compiled; }
		bool IsError()    const { return Error; }

		virtual bool Load(const char* FileName) { return false; }
		virtual bool Load(StandartProgram Program) { return false; }
		virtual bool Compile() { return false; }

		virtual ~ShaderProgram() {}
	};

	struct DefaultShaders
	{
		ShaderProgram* Final = nullptr;
		ShaderProgram* GaussBlur = nullptr;
		ShaderProgram* BloomBright = nullptr;
		ShaderProgram* Bloom = nullptr;
		ShaderProgram* Icon = nullptr;
		ShaderProgram* Skybox = nullptr;
		ShaderProgram* SkyboxCubemapGeneration = nullptr;
		ShaderProgram* IrradianceGeneration = nullptr;
		ShaderProgram* PrefilterGeneration = nullptr;
		ShaderProgram* IntegrationGeneration = nullptr;

		DefaultShaders();
		~DefaultShaders();
	};

}


