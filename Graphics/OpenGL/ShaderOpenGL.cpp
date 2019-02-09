#include <Graphics/OpenGL/ShaderOpenGL.h>
#include <Graphics/OpenGL/StandartShadersOpenGL.h>
#include <System/File.h>
#include <GL/glew.h>
#include <cctype>

namespace Columbus
{
	
	/*
	*
	* Shader loading functions
	*
	*/
	bool ShaderLoadFromFile(const char* FileName, char*& OutSource, ShaderBuilder& Builder, ShaderType Type)
	{
		File ShaderFile(FileName, "rt");
		if (!ShaderFile.IsOpened())
		{
			Log::Error("Shader not loaded: %s", FileName);
			return false;
		}

		char* TmpFile = new char[(uint32)ShaderFile.GetSize() + 1];
		memset(TmpFile, 0, (uint32)ShaderFile.GetSize() + 1);
		ShaderFile.ReadBytes(TmpFile, ShaderFile.GetSize());
		ShaderFile.Close();

		if (!Builder.Build(TmpFile, Type))
		{
			Log::Error("Shader not built: %s", FileName);
			return false;
		}

		delete[] TmpFile;

		if (Builder.ShaderSourceLength == 0)
		{
			Log::Error("Shader loading incorrect: %s", FileName);
			return false;
		}

		OutSource = new char[Builder.ShaderSourceLength + 1];
		memcpy(OutSource, Builder.ShaderSource, Builder.ShaderSourceLength);
		OutSource[Builder.ShaderSourceLength] = '\0';

		return true;
	}

	bool ShaderLoad(const char* Path, char*& OutSource, ShaderBuilder& Builder, ShaderType Type)
	{
		switch (Type)
		{
			case ShaderType::Vertex:
			{
				if (strcmp(Path, "STANDART_SKY_VERTEX") == 0)
				{
					unsigned int Length = strlen(gSkyboxVertexShader);
					OutSource = new char[Length + 1];
					memcpy(OutSource, gSkyboxVertexShader, Length);
					OutSource[Length] = '\0';
					return true;
				}
				else if (ShaderLoadFromFile(Path, OutSource, Builder, Type))
				{
					return true;
				}

				break;
			}

			case ShaderType::Fragment:
			{
				if (strcmp(Path, "STANDART_SKY_FRAGMENT") == 0)
				{
					unsigned int Length = strlen(gSkyboxFragmentShader);
					OutSource = new char[Length + 1];
					memcpy(OutSource, gSkyboxFragmentShader, Length);
					OutSource[Length] = '\0';
					return true;
				}
				else if (ShaderLoadFromFile(Path, OutSource, Builder, Type))
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
	bool ShaderGetError(const char* ShaderPath, int32 ShaderID)
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
			Log::Error("%s: %s", ShaderPath,  Error);

			delete[] Error;
			return true;
		}

		return false;
	}

	bool ShaderCompile(const char* ShaderPath, const char* ShaderSource, int32 ShaderID)
	{
		glShaderSource(ShaderID, 1, &ShaderSource, NULL);
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

	bool ShaderStageOpenGL::Load(const char* FileName, ShaderType InType)
	{
		delete[] ShaderPath;
		delete[] ShaderSource;

		uint32 tType;
		
		switch (InType)
		{
		case ShaderType::Vertex:   tType = GL_VERTEX_SHADER; break;
		case ShaderType::Fragment: tType = GL_FRAGMENT_SHADER; break;
		default: return false; break;
		}
		
		ID = glCreateShader(tType);

		if (ID == 0) return false;

		if (!ShaderLoad(FileName, ShaderSource, Builder, InType))
		{
			return false;
		}

		unsigned int Length = strlen(FileName);
		ShaderPath = new char[Length + 1];
		memcpy(ShaderPath, FileName, Length);
		ShaderPath[Length] = '\0';
		
		Type = InType;
		Loaded = true;
		Compiled = false;
		Error = false;
		Log::Success("Shader loaded: %s", FileName);

		return true;
	}

	bool ShaderStageOpenGL::Compile()
	{
		if (!Loaded)
		{
			Log::Error("Couldn't compile shader: Shader wasn't loaded");
			Compiled = false;
			Error = true;
			return false;
		}

		if (!IsValid())
		{
			Log::Error("Couldn't compile shader: Shader is invalid");
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
		Log::Success("Shader compiled: %s", ShaderPath);

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
			Stages[CurrentStage++] = Stage;
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

		for (auto& Stage : Stages)
		{
			if (Stage != nullptr)
			{
				if (!Stage->IsLoaded())
				{
					Loaded = false;
					return false;
				}
			}
		}

		Loaded = true;
		return true;
	}

	bool ShaderProgramOpenGL::Load(const char* FileName)
	{
		File ShaderFile(FileName, "rt");
		if (!ShaderFile.IsOpened())
		{
			Log::Error("Shader not loaded: %s", FileName);
			return false;
		}

		ShaderStage* Stage;

		Stage = new ShaderStageOpenGL();
		Stage->Load(FileName, ShaderType::Vertex);
		AddStage(Stage);

		Stage = new ShaderStageOpenGL();
		Stage->Load(FileName, ShaderType::Fragment);
		AddStage(Stage);

		constexpr int MaxCount = 4096;
		char* Line = new char[MaxCount];
		int Offset;
		char Name[256];
		int NameLength;
		int Number;

		while (!ShaderFile.IsEOF())
		{
			ShaderFile.ReadLine(Line, MaxCount);
			Offset = 0;

			while (isblank(*Line)) { Line++; Offset++; }

			if (strncmp(Line, "//@", 3) == 0)
			{
				Line += 3; Offset += 3;
				while (isblank(*Line)) { Line++; Offset++; }

				if (strncmp(Line, "Attribute", 9) == 0)
				{
					Line += 9; Offset += 9;
					memset(Name, 0, sizeof(Name));
					NameLength = 0;
					Number = 0;
					while (isblank(*Line)) { Line++; Offset++; }
					while (isalpha(*Line)) { Name[NameLength++] = *Line; Line++; Offset++; }
					while (isblank(*Line)) { Line++; Offset++; }
					while (isdigit(*Line)) { Number *= 10; Number += *Line - '0'; Line++; Offset++; }

					if (NameLength > 0 && Number >= 0)
					{
						char* Tmp = new char[NameLength + 1];
						memcpy(Tmp, Name, NameLength);
						Tmp[NameLength] = '\0';
						Attributes[CurrentAttribute++] = { Tmp, (uint32)Number };
					}
				}

				if (strncmp(Line, "Uniform", 7) == 0)
				{
					Line += 7; Offset += 7;
					memset(Name, 0, sizeof(Name));
					NameLength = 0;
					while (isblank(*Line)) { Line++; Offset++; }
					while (isalpha(*Line) || isdigit(*Line) || *Line == '.') { Name[NameLength++] = *Line; Line++; Offset++; }
					Name[NameLength] = '\0';

					if (NameLength > 0)
					{
						Uniforms[CurrentUniform] = new char[NameLength + 1];
						memcpy(Uniforms[CurrentUniform], Name, NameLength);
						Uniforms[CurrentUniform][NameLength] = '\0';
						CurrentUniform++;
					}
				}
			}

			Line -= Offset;
		}

		delete[] Line;

		for (auto& Stage : Stages)
		{
			if (Stage != nullptr)
			{
				if (!Stage->IsLoaded())
				{
					Loaded = false;
					return false;
				}
			}
		}

		Loaded = true;
		return true;
	}

	bool ShaderProgramOpenGL::Compile()
	{
		bool VertexShaderExists = false;
		bool FragmentShaderExists = false;

		for (auto& Stage : Stages)
		{
			if (Stage != nullptr)
			{
				if (Stage->GetType() == ShaderType::Vertex) VertexShaderExists = true;
				if (Stage->GetType() == ShaderType::Fragment) FragmentShaderExists = true;
			}
		}

		if (!VertexShaderExists || !FragmentShaderExists)
		{
			Log::Error("Couldn't compile Shader Program: Needs vertex and fragment shader");
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
					Log::Error("Couldn't compile Shader Program: One or more of the shader not compiled");
					Compiled = false;
					Error = true;
					return false;
				}
			}

			if (!Stage->IsValid())
			{
				Log::Error("Couldn't compile Shader Program: One or more of the shaders is invalid");
				Compiled = false;
				Error = true;
				return false;
			}

			glAttachShader(ID, static_cast<ShaderStageOpenGL*>(Stage)->GetID());
		}

		for (int i = 0; i < CurrentAttribute; i++)
		{
			glBindAttribLocation(ID, Attributes[i].Value, Attributes[i].Name);
		}

		glLinkProgram(ID);

		for (int i = 0; i < CurrentUniform; i++)
		{
			AddUniform(Uniforms[i]);
		}

		Compiled = true;
		Error = false;
		Log::Success("Shader program compiled");

		return true;
	}

	bool ShaderProgramOpenGL::AddUniform(const char* Name)
	{
		int32 Value = glGetUniformLocation(ID, Name);

		if (Value != -1)
		{
			UniformLocations[Name] = Value;
			FastUniformsMap[Name] = CurrentFastUniform;
			FastUniforms[CurrentFastUniform++] = Value;
			return true;
		}

		return false;
	}

	void ShaderProgramOpenGL::SetUniform1i(const char* Name, int Value) const
	{
		if (ID != 0 && Compiled)
		{
			auto Location = UniformLocations.find(Name);
			if (Location != UniformLocations.end())
			{
				glUniform1i(Location->second, Value);
			}
		}
	}

	void ShaderProgramOpenGL::SetUniform1f(const char* Name, float Value) const
	{
		if (ID != 0 && Compiled)
		{
			auto Location = UniformLocations.find(Name);
			if (Location != UniformLocations.end())
			{
				glUniform1f(Location->second, Value);
			}
		}
	}

	void ShaderProgramOpenGL::SetUniform2f(const char* Name, const Vector2& Value) const
	{
		if (ID != 0 && Compiled)
		{
			auto Location = UniformLocations.find(Name);
			if (Location != UniformLocations.end())
			{
				glUniform2f(Location->second, Value.X, Value.Y);
			}
		}
	}

	void ShaderProgramOpenGL::SetUniform3f(const char* Name, const Vector3& Value) const
	{
		if (ID != 0 && Compiled)
		{
			auto Location = UniformLocations.find(Name);

			if (Location != UniformLocations.end())
			{
				glUniform3f(Location->second, Value.X, Value.Y, Value.Z);
			}
		}
	}

	void ShaderProgramOpenGL::SetUniform4f(const char* Name, const Vector4& Value) const
	{
		if (ID != 0 && Compiled)
		{
			auto Location = UniformLocations.find(Name);
			if (Location != UniformLocations.end())
			{
				glUniform4f(Location->second, Value.X, Value.Y, Value.Z, Value.W);
			}
		}
	}

	void ShaderProgramOpenGL::SetUniformMatrix(const char* Name, const float* Value) const
	{
		if (ID != 0 && Compiled)
		{
			auto Location = UniformLocations.find(Name);
			if (Location != UniformLocations.end())
			{
				glUniformMatrix4fv(Location->second, 1, GL_FALSE, Value);
			}
		}
	}

	void ShaderProgramOpenGL::SetUniformArrayf(const char* Name, const float* Array, uint32 Size) const
	{
		if (ID != 0 && Compiled)
		{
			auto Location = UniformLocations.find(Name);
			if (Location != UniformLocations.end())
			{
				glUniform1fv(Location->second, Size, Array);
			}
		}
	}

	void ShaderProgramOpenGL::SetUniformTexture(const char* Name, TextureOpenGL* Tex, uint32 Sampler) const
	{
		if (ID != 0 && Compiled)
		{
			auto Location = UniformLocations.find(Name);
			if (Location != UniformLocations.end())
			{
				glActiveTexture(GL_TEXTURE0 + Sampler);
				glUniform1i(Location->second, Sampler);
				Tex->Bind();
			}
		}
	}

	int32 ShaderProgramOpenGL::GetFastUniform(const char* Name) const
	{
		auto Fast = FastUniformsMap.find(Name);
		return Fast != FastUniformsMap.end() ? Fast->second : -1;
	}

	void ShaderProgramOpenGL::SetUniform(int FastID, int Value) const
	{
		glUniform1i(FastUniforms[FastID], Value);
	}

	void ShaderProgramOpenGL::SetUniform(int FastID, float Value) const
	{
		glUniform1f(FastUniforms[FastID], Value);
	}

	void ShaderProgramOpenGL::SetUniform(int FastID, const Vector2& Value) const
	{
		glUniform2f(FastUniforms[FastID], Value.X, Value.Y);
	}

	void ShaderProgramOpenGL::SetUniform(int FastID, const Vector3& Value) const
	{
		glUniform3f(FastUniforms[FastID], Value.X, Value.Y, Value.Z);
	}

	void ShaderProgramOpenGL::SetUniform(int FastID, const Vector4& Value) const
	{
		glUniform4f(FastUniforms[FastID], Value.X, Value.Y, Value.Z, Value.W);
	}

	void ShaderProgramOpenGL::SetUniform(int FastID, uint32 Size, const float* Value) const
	{
		glUniform1fv(FastUniforms[FastID], Size, Value);
	}

	void ShaderProgramOpenGL::SetUniform(int FastID, bool Transpose, const Matrix& Mat) const
	{
		glUniformMatrix4fv(FastID, 1, Transpose ? GL_TRUE : GL_FALSE, &Mat.M[0][0]);
	}

	void ShaderProgramOpenGL::SetUniform(int FastID, TextureOpenGL* Tex, uint32 Sampler) const
	{
		glActiveTexture(GL_TEXTURE0 + Sampler);
		glUniform1i(FastUniforms[FastID], Sampler);
		Tex->Bind();
	}

	ShaderProgramOpenGL::~ShaderProgramOpenGL()
	{
		for (auto& Stage : Stages)
		{
			delete Stage;
		}

		for (auto& Attribute : Attributes)
		{
			delete[] Attribute.Name;
		}

		for (auto& Uniform : Uniforms)
		{
			delete[] Uniform;
		}

		glDeleteProgram(ID);
	}

}








