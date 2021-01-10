#pragma once

#include <memory>
#include <vector>
#include <string>
#include <Graphics/Types.h>

namespace Columbus
{

	enum class ShaderPropertyType
	{
		Float,
		Float2,
		Float3,
		Float4
	};

	struct ShaderProperty
	{
		std::string Name;
		std::string AlterName; // name which appears in the Inspector window
		ShaderPropertyType Type;

		ShaderProperty(std::string&& Name, std::string&& AlterName, ShaderPropertyType Type) :
			Name(std::move(Name)),
			AlterName(std::move(AlterName)),
			Type(Type) {}
	};

	class ShaderProgram
	{
	protected:
		bool Loaded;
		bool Compiled;
		bool Error;

		std::vector<ShaderProperty> _Properties;
	public:
		ShaderProgram() {}

		bool IsLoaded()   const { return Loaded; }
		bool IsCompiled() const { return Compiled; }
		bool IsError()    const { return Error; }

		std::vector<ShaderProperty> GetProperties() const { return _Properties; }

		virtual bool LoadFromMemory(const char* Source, const char* FilePath = "") = 0;
		virtual bool Load(const char* FileName) = 0;
		virtual bool Compile() = 0;

		virtual ~ShaderProgram() {}
	};

	struct DefaultShaders
	{
		std::unique_ptr<ShaderProgram> Error;
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


