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
			Skybox,
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

}


