#include <Graphics/OpenGL/ShaderOpenGL.h>
#include <Graphics/OpenGL/StandartShadersOpenGL.h>
#include <GL/glew.h>

#include <System/File.h>

#include <algorithm>
#include <fstream>
#include <sstream>

namespace Columbus
{

const char* CommonShaderHeader =
R"(
#version 330 core
#define Texture2D sampler2D
#define Texture3D sampler3D
#define TextureCube samplerCube

#define Texture2DMS sampler2DMS

#define saturate(x) clamp(x, 0, 1)

#define float2 vec2
#define float3 vec3
#define float4 vec4

#define int2 ivec2
#define int3 ivec3
#define int4 ivec4

#define float2x2 mat2x2
#define float2x3 mat2x3
#define float2x4 mat2x4
#define float3x2 mat3x2
#define float3x3 mat3x3
#define float3x4 mat3x4
#define float4x2 mat4x2
#define float4x3 mat4x3
#define float4x4 mat4x4

#if __VERSION__ < 130
	#define Sample2D(tex, uv) texture2D(tex, uv)
	#define Sample3D(tex, uv) texture3D(tex, uv)
	#define SampleCube(tex, uv) textureCube(tex, uv)

	#define Sample2DLod(tex, uv, lod) texture2DLod(tex, uv, lod)
	#define Sample3DLod(tex, uv, lod) texture3DLod(tex, uv, lod)
	#define SampleCubeLod(tex, uv, lod) textureCubeLod(tex, uv, lod)
#else
	#define Sample2D(tex, uv) texture(tex, uv)
	#define Sample3D(tex, uv) texture(tex, uv)
	#define SampleCube(tex, uv) texture(tex, uv)

	#define Sample2DLod(tex, uv, lod) textureLod(tex, uv, lod)
	#define Sample3DLod(tex, uv, lod) textureLod(tex, uv, lod)
	#define SampleCubeLod(tex, uv, lod) textureLod(tex, uv, lod)

	#define Sample2DMS(tex, uv, lod) texelFetch(tex, uv, lod)
#endif
)";

const char* VertexShaderHeader =
R"(
#define VertexShader
#define SV_Position gl_Position
#define SV_VertexID gl_VertexID
#line 1
)";

const char* FragmentShaderHeader =
R"(
#define FragmentShader
#define SV_Depth gl_FragDepth
#define SV_Position gl_FragCoord
layout(location = 0) out float4 RT0;
layout(location = 1) out float4 RT1;
layout(location = 2) out float4 RT2;
layout(location = 3) out float4 RT3;
#line 1
)";

const char* ScreenSpaceVertexShader =
R"(
out float2 var_UV;

const float2 Coord[6] = float2[](
	float2(-1, +1),
	float2(+1, +1),
	float2(+1, -1),
	float2(-1, -1),
	float2(-1, +1),
	float2(+1, -1)
);

const float2 UV[6] = float2[](
	float2(0, 1),
	float2(1, 1),
	float2(1, 0),
	float2(0, 0),
	float2(0, 1),
	float2(1, 0)
);

void main(void)
{
	SV_Position = float4(Coord[SV_VertexID], 0, 1);
	var_UV = UV[SV_VertexID];
}
)";

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

	static std::vector<std::string> TokenizeString(const std::string& str, const char* delims)
	{
		std::vector<std::string> tokens;
		size_t pos = 0;

		while (pos < str.length())
		{
			size_t offset = strcspn(str.c_str() + pos, delims);
			if (offset != 0)
			{
				tokens.push_back(str.substr(pos, offset));
			}
			pos += offset + 1;
		}

		return tokens;
	}

	ShaderProgramOpenGL::ShaderData ParseShader(const char* Source)
	{
		ShaderProgramOpenGL::ShaderData Data;

		std::stringstream f(Source);
		std::string line;

		int CurrentMode = -1;
		std::stringstream streams[2];

		while (std::getline(f, line))
		{
			if (line.find("#shader") != std::string::npos)
			{
				auto tokens = TokenizeString(line, " \t");

				if (tokens[1] == "vertex")
				{
					CurrentMode = 0;
					if (tokens.size() == 4 && tokens[2] == "=")
					{
						if (tokens[3] == "screen_space")
						{
							streams[CurrentMode] << ScreenSpaceVertexShader;
						}
					}
				}

				if (tokens[1] == "fragment")
				{
					CurrentMode = 1;
				}
			}
			else if (line.find("#uniform") != std::string::npos)
			{
				auto bracket = line.find('[');
				auto tokens = TokenizeString(line.substr(0, bracket), " \t");

				auto&& type = std::move(tokens[1]);
				auto&& name = std::move(tokens[2]);
				auto brackets = bracket != std::string::npos ? line.substr(bracket) : "";

				streams[CurrentMode] << "uniform " << type << ' ' << name << brackets << ";\n";
				Data.Uniforms.emplace_back(std::move(name));
			}
			else if (line.find("#attribute") != std::string::npos)
			{
				auto tokens = TokenizeString(line, " \t");

				auto&& type = std::move(tokens[1]);
				auto&& name = std::move(tokens[2]);
				auto&& slot = std::move(tokens[3]);

				streams[CurrentMode] << "in " << type << ' ' << name << ";\n";
				Data.Attributes.emplace_back(std::move(name), atoi(slot.c_str()));
			}
			else if (line.find("#include") != std::string::npos)
			{
				auto tokens = TokenizeString(line, " <>\"\t");

				if (CurrentMode == -1)
				{
					auto Included = ReadFile(tokens[1].c_str());
					for (auto& stream : streams) stream << Included << '\n';
				}
				else
				{
					streams[CurrentMode] << ReadFile(tokens[1].c_str()) << '\n';
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

		Data.VertexSource = CommonShaderHeader + std::string(VertexShaderHeader) + streams[0].str();
		Data.FragmentSource = CommonShaderHeader + std::string(FragmentShaderHeader) + streams[1].str();

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
		glGetShaderiv(ShaderID, GL_INFO_LOG_LENGTH, &Length);
		Error = new char[Length];
		glGetShaderInfoLog(ShaderID, Length, &Length, Error);

		if (Status == GL_FALSE)
		{
			Log::Error("%s shader (%s): %s", GetStringFromShaderType(Type), ShaderPath, Error);
		} else if (Length > 1)
		{
			Log::Warning("%s shader (%s): %s", GetStringFromShaderType(Type), ShaderPath, Error);
		}

		delete[] Error;
		return Status == GL_FALSE;
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
		#define STD_PROG_CASE(x) \
			case ShaderProgram::StandartProgram::x: \
				LoadFromMemory(g##x##Shader, #x); \
				break;

		switch (Program)
		{
		STD_PROG_CASE(ScreenSpace);
		STD_PROG_CASE(AutoExposure);
		STD_PROG_CASE(Tonemap);
		STD_PROG_CASE(ResolveMSAA);
		STD_PROG_CASE(GaussBlur);
		STD_PROG_CASE(BloomBright);
		STD_PROG_CASE(Bloom);
		STD_PROG_CASE(Vignette);
		STD_PROG_CASE(FXAA);
		STD_PROG_CASE(Icon);
		STD_PROG_CASE(EditorTools);
		STD_PROG_CASE(Skybox);
		STD_PROG_CASE(SkyboxCubemapGeneration);
		STD_PROG_CASE(IrradianceGeneration);
		STD_PROG_CASE(PrefilterGeneration);
		STD_PROG_CASE(IntegrationGeneration);
		}

		Log::Success("Default shader program loaded: %s", Path.c_str());

		Loaded = true;
		return true;
	}

	bool ShaderProgramOpenGL::LoadFromMemory(const char* Source, const char* FilePath)
	{
		Data = ParseShader(Source);
		Path = FilePath;

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

		char* _data = (char*)malloc(ShaderFile.GetSize() + 1);
		ShaderFile.ReadBytes(_data, ShaderFile.GetSize());
		_data[ShaderFile.GetSize()] = '\0';

		LoadFromMemory(_data);
		Path = FileName;

		free(_data);

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

	void ShaderProgramOpenGL::SetUniform(int FastID, uint32 Count, const float* Value) const
	{
		glUniform1fv(FastUniforms[FastID], Count, Value);
	}

	void ShaderProgramOpenGL::SetUniform(int FastID, uint32 Count, const Vector2* Value) const
	{
		glUniform2fv(FastUniforms[FastID], Count, (const float*)Value);
	}

	void ShaderProgramOpenGL::SetUniform(int FastID, uint32 Count, const Vector3* Value) const
	{
		glUniform3fv(FastUniforms[FastID], Count, (const float*)Value);
	}

	void ShaderProgramOpenGL::SetUniform(int FastID, uint32 Count, const Vector4* Value) const
	{
		glUniform4fv(FastUniforms[FastID], Count, (const float*)Value);
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


