#pragma once

#include <memory>

namespace Columbus
{

	enum class ShaderType
	{
		Vertex,
		Fragment
	};

	class ShaderProgram
	{
	protected:
		bool Loaded;
		bool Compiled;
		bool Error;
	public:
		ShaderProgram() {}

		bool IsLoaded()   const { return Loaded; }
		bool IsCompiled() const { return Compiled; }
		bool IsError()    const { return Error; }

		virtual bool LoadFromMemory(const char* Source, const char* FilePath = "") = 0;
		virtual bool Load(const char* FileName) = 0;
		virtual bool Compile() = 0;

		virtual ~ShaderProgram() {}
	};

	struct DefaultShaders
	{
		std::unique_ptr<ShaderProgram> ScreenSpace;
		std::unique_ptr<ShaderProgram> AutoExposure;
		std::unique_ptr<ShaderProgram> Tonemap;
		std::unique_ptr<ShaderProgram> ResolveMSAA;
		std::unique_ptr<ShaderProgram> GaussBlur;
		std::unique_ptr<ShaderProgram> BloomBright;
		std::unique_ptr<ShaderProgram> Bloom;
		std::unique_ptr<ShaderProgram> Vignette;
		std::unique_ptr<ShaderProgram> FXAA;
		std::unique_ptr<ShaderProgram> Icon;
		std::unique_ptr<ShaderProgram> EditorTools;
		std::unique_ptr<ShaderProgram> Skybox;
		std::unique_ptr<ShaderProgram> SkyboxCubemapGeneration;
		std::unique_ptr<ShaderProgram> IrradianceGeneration;
		std::unique_ptr<ShaderProgram> PrefilterGeneration;
		std::unique_ptr<ShaderProgram> IntegrationGeneration;

		DefaultShaders();
		~DefaultShaders();
	};

}


