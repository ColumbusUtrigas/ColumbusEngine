#include <Graphics/OpenGL/ShaderOpenGL.h>
#include <Graphics/OpenGL/StandartShadersOpenGL.h>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <GL/glew.h>

namespace Columbus
{
	
	/*
	*
	* Shader loading functions
	*
	*/
	bool ShaderLoadFromFile(std::string InFile, std::string& OutSource, ShaderBuilder& Builder, ShaderType Type)
	{
		std::ifstream File;
		File.open(InFile.c_str());

		if (!File.is_open())
		{
			Log::error("Shader not loaded: " + InFile);
			return false;
		}

		std::string TmpFile = std::string((std::istreambuf_iterator<char>(File)), std::istreambuf_iterator<char>());

		if (!Builder.Build(TmpFile, Type))
		{
			Log::error("Shader not built: " + InFile);
			return false;
		}

		if (Builder.ShaderSource.empty())
		{
			Log::error("Shader loading incorrect: " + InFile);
			return false;
		}

		OutSource = Builder.ShaderSource;

		return true;
	}

	bool ShaderLoad(std::string InPath, std::string& OutSource, ShaderBuilder& Builder, ShaderType Type)
	{
		switch (Type)
		{
			case ShaderType::Vertex:
			{
				if (InPath == "STANDART_SKY_VERTEX")
				{
					OutSource = gSkyboxVertexShader;
					return true;
				}
				else if (ShaderLoadFromFile(InPath, OutSource, Builder, Type))
				{
					return true;
				}

				break;
			}

			case ShaderType::Fragment:
			{
				if (InPath == "STANDART_SKY_FRAGMENT")
				{
					OutSource = gSkyboxFragmentShader;
					return true;
				}
				else if (ShaderLoadFromFile(InPath, OutSource, Builder, Type))
				{
					return true;
				}

				break;
			}
		}

		return false;
	}
	/*
	*
	* End of shader loading functions
	*
	*/

	/*
	*
	* Shader compilation functions
	*
	*/
	bool ShaderGetError(std::string ShaderPath, int32 ShaderID)
	{
		int32 Status = GL_TRUE;
		int32 Length = 0;
		char* Error = nullptr;

		glGetShaderiv(ShaderID, GL_COMPILE_STATUS, &Status);

		if (Status == GL_FALSE)
		{
			glGetShaderiv(ShaderID, GL_INFO_LOG_LENGTH, &Length);
			Error = new char[Length];
			glGetShaderInfoLog(ShaderID, Length, &Length, Error);
			Log::error(ShaderPath + ": " + Error);

			delete[] Error;
			return true;
		}

		return false;
	}

	bool ShaderCompile(std::string ShaderPath, std::string ShaderSource, int32 ShaderID)
	{
		auto Source = ShaderSource.c_str();
		glShaderSource(ShaderID, 1, &Source, NULL);
		glCompileShader(ShaderID);

		if (ShaderGetError(ShaderPath, ShaderID))
		{
			return false;
		}

		return true;
	}

	void ShaderCleanup(int32 VertexID, int32 FragmentID)
	{
		glDeleteShader(VertexID);
		glDeleteShader(FragmentID);
	}
	/*
	*
	* End of shader compilation functions
	*
	*/
	
	/*
	*
	* ShaderStageOpenGL implementation
	*
	*/
	ShaderStageOpenGL::ShaderStageOpenGL()
	{
		
	}

	bool ShaderStageOpenGL::IsValid() const
	{
		return ID != 0 && glIsShader(ID);
	}

	bool ShaderStageOpenGL::Load(std::string InPath, ShaderType InType)
	{
		uint32 tType;
		
		switch (InType)
		{
		case ShaderType::Vertex:   tType = GL_VERTEX_SHADER; break;
		case ShaderType::Fragment: tType = GL_FRAGMENT_SHADER; break;
		default: return false; break;
		}
		
		ID = glCreateShader(tType);

		if (ID == 0) return false;

		if (!ShaderLoad(InPath, ShaderSource, Builder, InType))
		{
			return false;
		}

		ShaderPath = InPath;
		Type = InType;
		Loaded = true;
		Compiled = false;
		Error = false;
		Log::success("Shader loaded: " + InPath);

		return true;
	}

	bool ShaderStageOpenGL::Compile()
	{
		if (!Loaded)
		{
			Log::error("Couldn't compile shader: Shader wasn't loaded");
			Compiled = false;
			Error = true;
			return false;
		}

		if (!IsValid())
		{
			Log::error("Couldn't compile shader: Shader is invalid");
			Compiled = false;
			Error = true;
			return false;
		}

		if (!ShaderCompile(ShaderPath, ShaderSource, ID))
		{
			glDeleteShader(ID);
			Compiled = false;
			Error = true;
			return false;
		}

		Compiled = true;
		Error = false;
		Log::success("Shader compiled: " + ShaderPath);

		return true;
	}

	uint32 ShaderStageOpenGL::GetID() const
	{
		return ID;
	}

	ShaderStageOpenGL::~ShaderStageOpenGL()
	{
		if (ID != 0)
		{
			glDeleteShader(ID);
		}
	}
	/*
	*
	* ShaderProgramOpenGL implementation
	*
	*/
	ShaderProgramOpenGL::ShaderProgramOpenGL()
	{
		ID = glCreateProgram();

		Compiled = false;
		Error = false;
	}

	void ShaderProgramOpenGL::Bind() const
	{
		glUseProgram(ID);
	}

	void ShaderProgramOpenGL::Unbind() const
	{
		glUseProgram(0);
	}

	void ShaderProgramOpenGL::AddStage(ShaderStage* Stage)
	{
		if (Stage != nullptr)
		{
			Stages.push_back(Stage);
		}
	}

	bool ShaderProgramOpenGL::Load(ShaderProgram::StandartProgram Program)
	{
		switch (Program)
		{
			case ShaderProgram::StandartProgram::Skybox:
			{
				ShaderStage* VertexStage = new ShaderStageOpenGL();
				ShaderStage* FragmentStage = new ShaderStageOpenGL();

				VertexStage->Load("STANDART_SKY_VERTEX", ShaderType::Vertex);
				FragmentStage->Load("STANDART_SKY_FRAGMENT", ShaderType::Fragment);

				AddStage(VertexStage);
				AddStage(FragmentStage);
			}
		}

		return true;
	}

	bool ShaderProgramOpenGL::Load(const std::string& FileName)
	{
		std::ifstream File;
		File.open(FileName.c_str());

		if (!File.is_open())
		{
			Log::error("Shader not loaded: " + FileName);
			return false;
		}

		//std::string TmpFile = std::string((std::istreambuf_iterator<char>(File)), std::istreambuf_iterator<char>());

		ShaderStage* Stage;

		Stage = new ShaderStageOpenGL();
		Stage->Load(FileName, ShaderType::Vertex);
		AddStage(Stage);

		Stage = new ShaderStageOpenGL();
		Stage->Load(FileName, ShaderType::Fragment);
		AddStage(Stage);

		std::string Line;

		while (!File.eof())
		{
			std::getline(File, Line);
			std::stringstream ISS(Line);

			char C;
			std::string Name;
			std::string Value;

			ISS >> C; if (C == '/')
			ISS >> C; if (C == '/')
			ISS >> C; if (C == '@')
			{
				ISS >> Name;
				if (Name == "Attribute")
				{
					ISS >> Name;
					ISS >> Value;
					AddAttribute(Name, std::atoi(Value.c_str()));
				}
				else if (Name == "Uniform")
				{
					ISS >> Name;
					Uniforms.push_back(Name);
				}
			}
		}

		return true;
	}

	bool ShaderProgramOpenGL::Compile()
	{
		if (std::find_if(Stages.begin(), Stages.end(), [](ShaderStage* InStage)->bool { return InStage->GetType() == ShaderType::Vertex; }) == Stages.end() ||
			std::find_if(Stages.begin(), Stages.end(), [](ShaderStage* InStage)->bool { return InStage->GetType() == ShaderType::Fragment; }) == Stages.end())
		{
			Log::error("Coldn't compile Shader Program: Needs vertex and fragment shader");
			Compiled = false;
			Error = true;
			return false;
		}

		for (auto Stage : Stages)
		{
			if (!Stage->IsCompiled())
			{
				if (!Stage->Compile())
				{
					Log::error("Couldn't compile Shader Program: One or more of the shader not compiled");
					Compiled = false;
					Error = true;
					return false;
				}
			}

			if (!Stage->IsValid())
			{
				Log::error("Couldn't compile Shader Program: One or more of the shaders is invalid");
				Compiled = false;
				Error = true;
				return false;
			}

			glAttachShader(ID, static_cast<ShaderStageOpenGL*>(Stage)->GetID());
		}

		for (auto& Attrib : Attributes)
		{
			glBindAttribLocation(ID, Attrib.Value, Attrib.Name.c_str());
		}

		glLinkProgram(ID);

		for (auto& Uniform : Uniforms)
		{
			AddUniform(Uniform);
		}

		Uniforms.clear();

		Compiled = true;
		Error = false;
		Log::success("Shader program compiled");

		return true;
	}

	bool ShaderProgramOpenGL::AddUniform(const std::string& Name)
	{
		int32 Value = glGetUniformLocation(ID, Name.c_str());

		if (Value != -1)
		{
			UniformLocations[Name] = Value;
			return true;
		}

		return false;
	}

	void ShaderProgramOpenGL::SetUniform1i(const std::string& Name, int Value) const
	{
		if (ID != 0 && Compiled)
		{
			auto Location = UniformLocations.find(Name);
			if (Location != UniformLocations.end())
			{
				glUniform1i(UniformLocations.at(Name), Value);
			}
		}
	}

	void ShaderProgramOpenGL::SetUniform1f(const std::string& Name, float Value) const
	{
		if (ID != 0 && Compiled)
		{
			auto Location = UniformLocations.find(Name);
			if (Location != UniformLocations.end())
			{
				glUniform1f(UniformLocations.at(Name), Value);
			}
		}
	}

	void ShaderProgramOpenGL::SetUniform2f(const std::string& Name, const Vector2& Value) const
	{
		if (ID != 0 && Compiled)
		{
			auto Location = UniformLocations.find(Name);
			if (Location != UniformLocations.end())
			{
				glUniform2f(UniformLocations.at(Name), Value.X, Value.Y);
			}
		}
	}

	void ShaderProgramOpenGL::SetUniform3f(const std::string& Name, const Vector3& Value) const
	{
		if (ID != 0 && Compiled)
		{
			auto Location = UniformLocations.find(Name);

			if (Location != UniformLocations.end())
			{
				glUniform3f(UniformLocations.at(Name), Value.X, Value.Y, Value.Z);
			}
		}
	}

	void ShaderProgramOpenGL::SetUniform4f(const std::string& Name, const Vector4& Value) const
	{
		if (ID != 0 && Compiled)
		{
			auto Location = UniformLocations.find(Name);
			if (Location != UniformLocations.end())
			{
				glUniform4f(UniformLocations.at(Name), Value.X, Value.Y, Value.Z, Value.W);
			}
		}
	}

	void ShaderProgramOpenGL::SetUniformMatrix(const std::string& Name, const float* Value) const
	{
		if (ID != 0 && Compiled)
		{
			auto Location = UniformLocations.find(Name);
			if (Location != UniformLocations.end())
			{
				glUniformMatrix4fv(UniformLocations.at(Name), 1, GL_FALSE, Value);
			}
		}
	}

	void ShaderProgramOpenGL::SetUniformArrayf(const std::string& Name, const float* Array, uint32 Size) const
	{
		if (ID != 0 && Compiled)
		{
			auto Location = UniformLocations.find(Name);
			if (Location != UniformLocations.end())
			{
				glUniform1fv(UniformLocations.at(Name), Size, Array);
			}
		}
	}

	ShaderProgramOpenGL::~ShaderProgramOpenGL()
	{
		glDeleteProgram(ID);
	}

}








