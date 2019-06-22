#include <Graphics/OpenGL/ShaderOpenGL.h>
#include <Graphics/OpenGL/StandartShadersOpenGL.h>
#include <Graphics/ShaderBuilder.h>
#include <GL/glew.h>

#include <System/File.h>

#include <algorithm>
#include <fstream>
#include <sstream>

namespace Columbus
{

	static const char* GetStringFromShaderType(ShaderType Type)
	{
		switch (Type)
		{
		case ShaderType::Vertex:   return "Vertex";
		case ShaderType::Fragment: return "Fragment";
		}

		return "";
	}
	
	/*
	*
	* Shader loading functions
	*
	*/

	static std::string ReadFile(const char* FileName)
	{
		std::ifstream f(FileName);
		std::string line;
		std::stringstream stream;

		while (std::getline(f, line))
		{
			if (line.find("#include") != std::string::npos)
			{
				auto substr = line.substr(line.find("#include") + 8);
				size_t pos = 0;
				std::string path;
				while  (isspace(substr[pos]) && pos < substr.size()) pos++;
				while (!isspace(substr[pos]) && pos < substr.size())
				{
					if (substr[pos] != '"') path += substr[pos];
					pos++;
				}

				stream << ReadFile(path.c_str()) << '\n';
			}
			else
			{
				stream << line << '\n';
			}
		}

		return stream.str();
	}

	ShaderProgramOpenGL::ShaderData ParseShader(const char* FileName)
	{
		ShaderProgramOpenGL::ShaderData Data;

		std::ifstream f(FileName);
		std::string line;

		int CurrentMode = -1;
		std::stringstream streams[2];

		while (std::getline(f, line))
		{
			if (line.find("#shader") != std::string::npos)
			{
				if (line.find("vertex")   != std::string::npos) CurrentMode = 0;
				if (line.find("fragment") != std::string::npos) CurrentMode = 1;
			}
			else if (line.find("#uniform") != std::string::npos)
			{
				auto substr = line.substr(line.find("#uniform") + 8);
				size_t pos = 0;
				std::string type, name, brackets;
				while  (isspace(substr[pos]) && pos < substr.size()) pos++;
				while (!isspace(substr[pos]) && pos < substr.size()) type += substr[pos++];
				while  (isspace(substr[pos]) && pos < substr.size()) pos++;
				while (!isspace(substr[pos]) && pos < substr.size() && substr[pos] != '[') name += substr[pos++];
				while  (isspace(substr[pos]) && pos < substr.size()) pos++;
				while (substr[pos] != '\n' && pos < substr.size()) brackets += substr[pos++];

				streams[CurrentMode] << "uniform " << type << ' ' << name << brackets << ';' << '\n';

				Data.Uniforms.emplace_back(std::move(name));
			}
			else if (line.find("#attribute") != std::string::npos)
			{
				auto substr = line.substr(line.find("#attribute") + 10);
				size_t pos = 0;
				std::string type, name, slot;
				while  (isspace(substr[pos]) && pos < substr.size()) pos++;
				while (!isspace(substr[pos]) && pos < substr.size()) type += substr[pos++];
				while  (isspace(substr[pos]) && pos < substr.size()) pos++;
				while (!isspace(substr[pos]) && pos < substr.size()) name += substr[pos++];
				while  (isspace(substr[pos]) && pos < substr.size()) pos++;
				while (!isspace(substr[pos]) && pos < substr.size()) slot += substr[pos++];

				streams[CurrentMode] << "in " << type << ' ' << name << ';' << '\n';

				Data.Attributes.emplace_back(std::move(name), atoi(slot.c_str()));
			}
			else if (line.find("#include") != std::string::npos)
			{
				auto substr = line.substr(line.find("#include") + 8);
				size_t pos = 0;
				std::string path;
				while  (isspace(substr[pos]) && pos < substr.size()) pos++;
				while (!isspace(substr[pos]) && pos < substr.size())
				{
					if (substr[pos] != '"') path += substr[pos];
					pos++;
				}

				if (CurrentMode == -1)
				{
					auto Included = ReadFile(path.c_str());
					for (auto& stream : streams) stream << Included << '\n';
				}
				else
				{
					streams[CurrentMode] << ReadFile(path.c_str()) << '\n';
				}
			}
			else
			{
				if (CurrentMode == -1)
				{
					for (auto& stream : streams) stream << line << '\n';
				}
				else
				{
					streams[CurrentMode] << line << '\n';
				}
			}
		}

		ShaderBuilder Builder;

		Builder.Build(streams[0].str().c_str(), ShaderType::Vertex);
		Data.VertexSource   = Builder.ShaderSource;

		Builder.Build(streams[1].str().c_str(), ShaderType::Fragment);
		Data.FragmentSource = Builder.ShaderSource;

		return Data;
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
	static bool ShaderGetError(const char* ShaderPath, int32 ShaderID, ShaderType Type)
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
			Log::Error("%s shader (%s): %s: %s", GetStringFromShaderType(Type), ShaderPath, ShaderPath,  Error);

			delete[] Error;
			return true;
		}

		return false;
	}

	static bool ShaderCompile(const char* ShaderPath, const char* ShaderSource, int32 ShaderID, ShaderType Type)
	{
		glShaderSource(ShaderID, 1, &ShaderSource, nullptr);
		glCompileShader(ShaderID);

		if (ShaderGetError(ShaderPath, ShaderID, Type))
		{
			return false;
		}

		return true;
	}
	/*
	*
	* End of shader compilation functions
	*
	*/

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

	bool ShaderProgramOpenGL::Load(ShaderProgram::StandartProgram Program)
	{
		switch (Program)
		{
			case ShaderProgram::StandartProgram::ScreenSpace:
			{
				Data.VertexSource = gScreenSpaceVertexShader;
				Data.FragmentSource = gScreenSpaceFragmentShader;

				Data.Attributes.emplace_back("Pos", 0);
				Data.Attributes.emplace_back("UV", 1);
				Data.Uniforms.emplace_back("BaseTexture");

				Path = "ScreenSpace";

				Log::Success("Default shader program loaded: ScreenSpace");
				break;
			}

			case ShaderProgram::StandartProgram::Tonemap:
			{
				Data.VertexSource = gScreenSpaceVertexShader;
				Data.FragmentSource = gTonemapFragmentShader;

				Data.Attributes.emplace_back("Pos", 0);
				Data.Attributes.emplace_back("UV", 1);
				Data.Uniforms.emplace_back("BaseTexture");
				Data.Uniforms.emplace_back("Exposure");
				Data.Uniforms.emplace_back("Gamma");

				Path = "Tonemap";

				Log::Success("Default shader program loaded: Tonemap");
				break;
			}

			case ShaderProgram::StandartProgram::GaussBlur:
			{
				Data.VertexSource = gScreenSpaceVertexShader;
				Data.FragmentSource = gGaussBlurFragmentShader;

				Data.Attributes.emplace_back("Pos", 0);
				Data.Attributes.emplace_back("UV", 1);
				Data.Uniforms.emplace_back("BaseTexture");
				Data.Uniforms.emplace_back("Horizontal");
				Data.Uniforms.emplace_back("Radius");

				Path = "GaussBlur";

				Log::Success("Default shader program loaded: GaussBlur");
				break;
			}

			case ShaderProgram::StandartProgram::BloomBright:
			{
				Data.VertexSource = gScreenSpaceVertexShader;
				Data.FragmentSource = gBloomBrightFragmentShader;

				Data.Attributes.emplace_back("Pos", 0);
				Data.Attributes.emplace_back("UV", 1);
				Data.Uniforms.emplace_back("BaseTexture");
				Data.Uniforms.emplace_back("Treshold");

				Path = "BloomBright";

				Log::Success("Default shader program loaded: BloomBright");
				break;
			}

			case ShaderProgram::StandartProgram::Bloom:
			{
				Data.VertexSource = gScreenSpaceVertexShader;
				Data.FragmentSource = gBloomFragmentShader;

				Data.Attributes.emplace_back("Pos", 0);
				Data.Attributes.emplace_back("UV", 1);
				Data.Uniforms.emplace_back("BaseTexture");
				Data.Uniforms.emplace_back("Blur");
				Data.Uniforms.emplace_back("Intensity");

				Path = "Bloom";

				Log::Success("Default shader program loaded: Bloom");
				break;
			}

			case ShaderProgram::StandartProgram::Vignette:
			{
				Data.VertexSource = gScreenSpaceVertexShader;
				Data.FragmentSource = gVignetteFragmentShader;

				Data.Attributes.emplace_back("Pos", 0);
				Data.Attributes.emplace_back("UV", 1);
				Data.Uniforms.emplace_back("Color");
				Data.Uniforms.emplace_back("Center");
				Data.Uniforms.emplace_back("Intensity");
				Data.Uniforms.emplace_back("Smoothness");
				Data.Uniforms.emplace_back("Radius");

				Path = "Vignette";

				Log::Success("Default shader program loaded: Vignette");
				break;
			}

			case ShaderProgram::StandartProgram::FXAA:
			{
				Data.VertexSource = gScreenSpaceVertexShader;
				Data.FragmentSource = gFXAAFragmentShader;

				Data.Attributes.emplace_back("Pos", 0);
				Data.Attributes.emplace_back("UV", 1);
				Data.Uniforms.emplace_back("BaseTexture");
				Data.Uniforms.emplace_back("Resolution");

				Path = "FXAA";

				Log::Success("Default shader program loaded: FXAA");
				break;
			}

			case ShaderProgram::StandartProgram::Icon:
			{
				Data.VertexSource = gIconVertexShader;
				Data.FragmentSource = gIconFragmentShader;

				Data.Attributes.emplace_back("Position", 0);
				Data.Attributes.emplace_back("UV", 1);
				Data.Uniforms.emplace_back("Pos");
				Data.Uniforms.emplace_back("Size");
				Data.Uniforms.emplace_back("Texture");

				Path = "Icon";

				Log::Success("Default shader program loaded: Icon");
				break;
			}

			case ShaderProgram::StandartProgram::Skybox:
			{
				Data.VertexSource = gSkyboxVertexShader;
				Data.FragmentSource = gSkyboxFragmentShader;

				Data.Uniforms.emplace_back("ViewProjection");
				Data.Uniforms.emplace_back("Skybox");

				Path = "Skybox";

				Log::Success("Default shader program loaded: Skybox");
				break;
			}

			case ShaderProgram::StandartProgram::SkyboxCubemapGeneration:
			{
				Data.VertexSource = gSkyboxCubemapGenerationVertexShader;
				Data.FragmentSource = gSkyboxCubemapGenerationFragmentShader;

				Data.Uniforms.emplace_back("Projection");
				Data.Uniforms.emplace_back("View");
				Data.Uniforms.emplace_back("BaseMap");

				Path = "SkyboxCubemapGeneration";

				Log::Success("Default shader program loaded: SkyboxCubemapGeneration");
				break;
			}

			case ShaderProgram::StandartProgram::IrradianceGeneration:
			{
				Data.VertexSource = gIrradianceGenerationVertexShader;
				Data.FragmentSource = gIrradianceGenerationFragmentShader;

				Data.Attributes.emplace_back("Position", 0);
				Data.Uniforms.emplace_back("Projection");
				Data.Uniforms.emplace_back("View");
				Data.Uniforms.emplace_back("EnvironmentMap");

				Path = "IrradianceGeneration";

				Log::Success("Default shader program loaded: IrradianceGeneration");
				break;
			}

			case ShaderProgram::StandartProgram::PrefilterGeneration:
			{
				Data.VertexSource = gPrefilterGenerationVertexShader;
				Data.FragmentSource = gPrefilterGenerationFragmentShader;

				Data.Attributes.emplace_back("Position", 0);
				Data.Uniforms.emplace_back("Projection");
				Data.Uniforms.emplace_back("View");
				Data.Uniforms.emplace_back("Roughness");
				Data.Uniforms.emplace_back("EnvironmentMap");

				Path = "PrefilterGeneration";

				Log::Success("Default shader program loaded: PrefilterGeneration");
				break;
			}

			case ShaderProgram::StandartProgram::IntegrationGeneration:
			{
				Data.Attributes.emplace_back("Position", 0);
				Data.Attributes.emplace_back("Texcoord", 1);
				Data.VertexSource = gIntegrationGenerationVertexShader;
				Data.FragmentSource = gIntegrationGenerationFragmentShader;

				Path = "IntegrationGeneration";

				Log::Success("Default shader program loaded: IntegrationGeneration");
				break;
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

		Data = ParseShader(FileName);
		Path = FileName;

		Log::Success("Shader program loaded:   %s", FileName);

		Loaded = true;
		return true;
	}

	bool ShaderProgramOpenGL::Compile()
	{
		bool VertexShaderExists = !Data.VertexSource.empty();
		bool FragmentShaderExists = !Data.FragmentSource.empty();

		if (!VertexShaderExists || !FragmentShaderExists)
		{
			Log::Error("Couldn't compile Shader Program: Needs vertex and fragment shader");
			Compiled = false;
			Error = true;
			return false;
		}

		VertexShader   = glCreateShader(GL_VERTEX_SHADER);
		FragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

		bool VertexCompilation   = ShaderCompile(Path.c_str(), Data.VertexSource.c_str(), VertexShader, ShaderType::Vertex);
		bool FragmentCompilation = ShaderCompile(Path.c_str(), Data.FragmentSource.c_str(), FragmentShader, ShaderType::Fragment);

		if (!VertexCompilation || !FragmentCompilation)
		{
			Log::Error("Couldn't compile Shader Program: One or more of the shaders not compiled");
			Compiled = false;
			Error = true;
			return false;
		}

		glAttachShader(ID, VertexShader);
		glAttachShader(ID, FragmentShader);

		for (const auto& Attribute : Data.Attributes)
		{
			glBindAttribLocation(ID, Attribute.Slot, Attribute.Name.c_str());
		}

		glLinkProgram(ID);
		glValidateProgram(ID);

		for (const auto& Uniform : Data.Uniforms)
		{
			AddUniform(Uniform.c_str());
		}

		glDeleteShader(VertexShader);
		glDeleteShader(FragmentShader);

		Compiled = true;
		Error = false;
		Log::Success("Shader program compiled: %s", Path.c_str());

		return true;
	}

	bool ShaderProgramOpenGL::AddUniform(const char* Name)
	{
		int32 Value = glGetUniformLocation(ID, Name);

		if (Value != -1)
		{
			FastUniformsMap[Name] = CurrentFastUniform;
			FastUniforms[CurrentFastUniform++] = Value;
			return true;
		}

		return false;
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
		glUniformMatrix4fv(FastUniforms[FastID], 1, Transpose ? GL_TRUE : GL_FALSE, &Mat.M[0][0]);
	}

	void ShaderProgramOpenGL::SetUniform(int FastID, TextureOpenGL* Tex, uint32 Sampler) const
	{
		glActiveTexture(GL_TEXTURE0 + Sampler);
		glUniform1i(FastUniforms[FastID], Sampler);
		Tex->Bind();
	}

	ShaderProgramOpenGL::~ShaderProgramOpenGL()
	{
		glDeleteProgram(ID);
	}

}


