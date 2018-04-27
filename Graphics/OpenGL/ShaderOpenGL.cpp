#include <Graphics/OpenGL/ShaderOpenGL.h>
#include <GL/glew.h>
#include <fstream>

namespace Columbus
{
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//STANDART SKYBOX VERTEX SHADER
	const std::string gSkyVertexShader =
		"#version 130\n"
		"attribute vec3 aPos;\n"
		"attribute vec2 aUV;\n"
		"attribute vec3 aNorm;\n"
		"varying vec3 texCoord;\n"
		"uniform mat4 uView;\n"
		"uniform mat4 uProjection;\n"
		"void main()\n"
		"{\n"
		"gl_Position = uProjection * uView * vec4(aPos, 1.0);\n"
		"texCoord = aPos;\n"
		"}\n";
	//////////////////////////////////////////////////////////////////////////////
	//STANDART SKYBOX FRAGMENT SHADER
	const std::string gSkyFragmentShader =
		"#version 130\n"
		"varying vec3 texCoord;\n"
		"uniform samplerCube uSkybox;\n"
		"void main()\n"
		"{\n"
		"gl_FragColor = textureCube(uSkybox, texCoord);\n"
		"}\n";
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
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
					OutSource = gSkyVertexShader;
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
					OutSource = gSkyFragmentShader;
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
	ShaderOpenGL::ShaderOpenGL()
	{
		ID = glCreateProgram();
	}

	bool ShaderOpenGL::Load(std::string InVert, std::string InFrag)
	{
		std::string VertSource;
		std::string FragSource;

		if (!ShaderLoad(InVert, VertSource, Builder, ShaderType::Vertex)) return false;
		if (!ShaderLoad(InFrag, FragSource, Builder, ShaderType::Fragment)) return false;

		VertShaderPath = InVert;
		FragShaderPath = InFrag;

		VertShaderSource = VertSource;
		FragShaderSource = FragSource;
		Loaded = true;
		Compiled = false;

		Log::success("Shader loaded: " + InVert);
		Log::success("Shader loaded: " + InFrag);

		return true;
	}
	
	bool ShaderOpenGL::Compile()
	{
		if (!Loaded) return false;

		uint32 VertexID = glCreateShader(GL_VERTEX_SHADER);
		uint32 FragmentID = glCreateShader(GL_FRAGMENT_SHADER);

		if (!ShaderCompile(VertShaderPath, VertShaderSource, VertexID))   { ShaderCleanup(VertexID, FragmentID); return false; }
		if (!ShaderCompile(FragShaderPath, FragShaderSource, FragmentID)) { ShaderCleanup(VertexID, FragmentID); return false; }

		glAttachShader(ID, VertexID);
		glAttachShader(ID, FragmentID);

		for (auto Attrib : Attributes)
		{
			glBindAttribLocation(ID, Attrib.Value, Attrib.Name.c_str());
		}

		glLinkProgram(ID);

		ShaderCleanup(VertexID, FragmentID);
		Compiled = true;

		Log::success("Shader compiled: " + VertShaderPath);
		Log::success("Shader compiled: " + FragShaderPath);

		return true;
	}
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	void ShaderOpenGL::Bind() const
	{
		if (Compiled)
		{
			glUseProgram(ID);
		}
	}
	
	void ShaderOpenGL::Unbind() const
	{
		glUseProgram(0);
	}
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	void ShaderOpenGL::SetUniform1i(std::string Name, int Value) const
	{
		if (ID != 0 && Compiled) glUniform1i(glGetUniformLocation(ID, Name.c_str()), Value);
	}

	void ShaderOpenGL::SetUniform1f(std::string Name, float Value) const
	{
		if (ID != 0 && Compiled) glUniform1f(glGetUniformLocation(ID, Name.c_str()), Value);
	}

	void ShaderOpenGL::SetUniform2f(std::string Name, Vector2 Value) const
	{
		if (ID != 0 && Compiled) glUniform2f(glGetUniformLocation(ID, Name.c_str()), Value.X, Value.Y);
	}

	void ShaderOpenGL::SetUniform3f(std::string Name, Vector3 Value) const
	{
		if (ID != 0 && Compiled) glUniform3f(glGetUniformLocation(ID, Name.c_str()), Value.X, Value.Y, Value.Z);
	}

	void ShaderOpenGL::SetUniform4f(std::string Name, Vector4 Value) const
	{
		if (ID != 0 && Compiled) glUniform4f(glGetUniformLocation(ID, Name.c_str()), Value.x, Value.y, Value.z, Value.w);
	}
	
	void ShaderOpenGL::SetUniformMatrix(std::string Name, const float* Value) const
	{
		if (ID != 0 && Compiled) glUniformMatrix4fv(glGetUniformLocation(ID, Name.c_str()), 1, GL_FALSE, Value);
	}
	
	void ShaderOpenGL::SetUniformArrayf(std::string Name, const float* Array, uint32 Size) const
	{
		if (ID != 0 && Compiled) glUniform1fv(glGetUniformLocation(ID, Name.c_str()), Size, Array);
	}
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	ShaderOpenGL::~ShaderOpenGL()
	{
		glDeleteProgram(ID);
	}

}








