#include <Graphics/OpenGL/ShaderOpenGL.h>
#include <GL/glew.h>

#include <System/File.h>

#include <algorithm>
#include <fstream>
#include <sstream>
#include <regex>

namespace Columbus
{

const char* CommonShaderHeader =
R"(
#version 430 core
#define Texture2D sampler2D
#define Texture3D sampler3D
#define TextureCube samplerCube

#define Texture2DMS sampler2DMS
#define Texture2DShadow sampler2DShadow

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
	#define Sample2DShadow(tex, uvc) shadow2D(tex, uvc)
#else
	#define Sample2D(tex, uv) texture(tex, uv)
	#define Sample3D(tex, uv) texture(tex, uv)
	#define SampleCube(tex, uv) texture(tex, uv)

	#define Sample2DLod(tex, uv, lod) textureLod(tex, uv, lod)
	#define Sample3DLod(tex, uv, lod) textureLod(tex, uv, lod)
	#define SampleCubeLod(tex, uv, lod) textureLod(tex, uv, lod)

	#define Sample2DMS(tex, uv, lod) texelFetch(tex, uv, lod)
	#define Sample2DShadow(tex, uvc) texture(tex, uvc)
#endif
)";

const char* VertexShaderHeader =
R"(
#define VertexShader
#define SV_Position gl_Position
#define SV_VertexID gl_VertexID
#line 1
)";

const char* PixelShaderHeader =
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
		case ShaderType::Vertex: return "Vertex";
		case ShaderType::Pixel:  return "Pixel";
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

				stream << ReadFile(path.c_str()) << std::endl;
			}
			else
			{
				stream << line << '\n';
			}
		}

		return stream.str();
	}

	#define CHECK_ERROR(exp, pos, fmt, first_param) \
		if (exp) { \
			char err_msg[512] = {}; \
			snprintf(err_msg, 512, fmt, first_param); \
			Data.Errors.emplace_back(line_num, pos, err_msg); \
			err_occured = true; \
		}

	#define CHECK_UNIFORM_ERRORS() \
		CHECK_ERROR(!std::regex_match(type, regex_uniform_types), \
			match_uniform.position(1) + 1, \
			"Invalid uniform type '%s'", \
			type.c_str());

	#define CHECK_ATTRIBUTE_ERRORS() \
		CHECK_ERROR(CurrentMode != MODE_VERTEX, \
			line.find("#attribute") + 1, \
			"Vertex attributes available only in vertex shader", ""); \
		\
		CHECK_ERROR(!std::regex_match(type, regex_attribute_types), \
			match_attribute.position(1) + 1, \
			"Invalid attribute type '%s'", \
			type.c_str()); \
		\
		CHECK_ERROR(slot_num < 0 || slot_num > 15, \
			match_attribute.position(3) + 1, \
			"Invalid slot '%s': It should be a number in range [0;15]", \
			slot.c_str()); \

	ShaderProgramOpenGL::ShaderData ParseShader(const char* Source)
	{
		ShaderProgramOpenGL::ShaderData Data;

		std::stringstream f(Source);
		std::string line;
		size_t line_num = 0;
		bool err_occured = false;

		enum
		{
			MODE_NONE = -1,
			MODE_VERTEX = 0,
			MODE_PIXEL = 1
		} CurrentMode = MODE_NONE;

		std::stringstream streams[2];

		std::regex regex_uniform_types("\\b((bool|int|float)[2-4]?(x[2-4])?|Texture(2D|3D|Cube|2DMS|2DShadow))\\b");
		std::regex regex_attribute_types("\\b(float[2-4]?)\\b");

		std::regex regex_shader("\\s*#shader\\s+(vertex|pixel)\\s*(=\\s*(.+))?\\s*");
		std::regex regex_uniform("\\s*#uniform\\s+(\\w[\\w\\d_]*)\\s+(\\w[\\w\\d_]*)\\s*(\\[(.+)\\])?\\s*");
		std::regex regex_attribute("\\s*#attribute\\s+(\\w[\\w\\d_]*)\\s+(\\w[\\w\\d_]*)\\s+(\\d+)\\s*");
		std::regex regex_include("\\s*(#include)\\s*(\"(.*)\"|<(.*)>)\\s*");

		std::unordered_map<std::string, ShaderPropertyType> type_map{
			{ "float", ShaderPropertyType::Float },
			{ "float2", ShaderPropertyType::Float2 },
			{ "float3", ShaderPropertyType::Float3 },
			{ "float4", ShaderPropertyType::Float4 }
		};

		while (std::getline(f, line))
		{
			std::smatch match_shader,
			            match_uniform,
			            match_attribute,
			            match_include;

			line_num++;

			if (std::regex_match(line, match_shader, regex_shader))
			{
				if (match_shader.str(1) == "vertex")
				{
					CurrentMode = MODE_VERTEX;
					if (!match_shader.str(3).empty())
					{
						if (match_shader.str(3) == "screen_space")
						{
							streams[CurrentMode] << ScreenSpaceVertexShader;
						}
						else
						{
							CHECK_ERROR(false,
								match_shader.position(3) + 1,
								"Invalid default vertex shader", "");
						}
					}
				}
				else if (match_shader.str(1) == "pixel")
				{
					CurrentMode = MODE_PIXEL;
				}
			}
			else if (std::regex_match(line, match_uniform, regex_uniform))
			{
				auto type = match_uniform.str(1);
				auto name = match_uniform.str(2);
				auto brackets = match_uniform.str(3);

				CHECK_UNIFORM_ERRORS();

				auto prop_type = ShaderPropertyType::Float;
				auto t = type_map.find(type);
				if (t != type_map.end())
				{
					prop_type = t->second;
				}

				streams[CurrentMode] << "uniform " << type << ' ' << name << brackets << ";\n";
				Data.Uniforms.emplace_back(std::move(name), "", prop_type);
			}
			else if (std::regex_match(line, match_attribute, regex_attribute))
			{
				auto type = match_attribute.str(1);
				auto name = match_attribute.str(2);
				auto slot = match_attribute.str(3);
				int slot_num = atoi(slot.c_str());

				CHECK_ATTRIBUTE_ERRORS();

				streams[CurrentMode] << "in " << type << ' ' << name << ";\n";
				Data.Attributes.emplace_back(std::move(name), slot_num);
			}
			else if (std::regex_match(line, match_include, regex_include))
			{
				if (CurrentMode == MODE_NONE)
				{
					auto Included = ReadFile(match_include.str(3).c_str());
					for (auto& stream : streams) stream << Included << '\n';
				}
				else
				{
					streams[CurrentMode] << ReadFile(match_include.str(3).c_str()) << '\n';
				}
			}
			else
			{
				if (CurrentMode == MODE_NONE)
				{
					for (auto& stream : streams) stream << line << '\n';
				}
				else
				{
					streams[CurrentMode] << line << '\n';
				}
			}
		}

		if (err_occured) return Data;

		Data.VertexSource = CommonShaderHeader + std::string(VertexShaderHeader) + streams[MODE_VERTEX].str();
		Data.FragmentSource = CommonShaderHeader + std::string(PixelShaderHeader) + streams[MODE_PIXEL].str();

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

	bool ShaderProgramOpenGL::LoadFromMemory(const char* Source, const char* FilePath)
	{
		Data = ParseShader(Source);
		Path = FilePath;

		if (!Data.Errors.empty())
		{
			for (const auto& err : Data.Errors)
				Log::Error("%s:%zu:%zu: %s", FilePath, err.Line, err.Position, err.Message.c_str());

			Loaded = false;
			return false;
		}

		Log::Success("Shader program loaded:   %s", FilePath);
		Loaded = true;
		_Properties = Data.Uniforms;
		return true;
	}

	bool ShaderProgramOpenGL::Load(const char* FileName)
	{
		std::ifstream t(FileName);
		if (!t.is_open())
		{
			Log::Error("Shader not loaded: %s", FileName);
			return false;
		}

		std::string str((std::istreambuf_iterator<char>(t)),
			             std::istreambuf_iterator<char>());

		bool result = LoadFromMemory(str.c_str(), FileName);

		return result;
	}

	bool ShaderProgramOpenGL::Compile()
	{
		if (Loaded && Compiled)
		{
			Log::Warning("Shader program already compiled, aborting: %s", Path.c_str());
			return false;
		}

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
		bool FragmentCompilation = ShaderCompile(Path.c_str(), Data.FragmentSource.c_str(), FragmentShader, ShaderType::Pixel);

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
			AddUniform(Uniform.Name.c_str());
		}

		glDeleteShader(VertexShader);
		glDeleteShader(FragmentShader);

		Compiled = true;
		Error = false;
		Log::Success("Shader program compiled: %s", Path.c_str());

		return true;
	}

	bool ShaderProgramOpenGL::AddUniform(const std::string& Name)
	{
		int32 Value = glGetUniformLocation(ID, Name.c_str());

		if (Value != -1)
		{
			FastUniformsMap[Name] = CurrentFastUniform;
			FastUniforms[CurrentFastUniform++] = Value;
			return true;
		}

		return false;
	}

	int32 ShaderProgramOpenGL::GetFastUniform(const std::string& Name) const
	{
		auto Fast = FastUniformsMap.find(Name);
		return Fast != FastUniformsMap.end() ? Fast->second : -1;
	}

	#define _SET_UNIFORM(func) \
		auto id = GetFastUniform(Name); \
		/*COLUMBUS_ASSERT_MESSAGE(id != -1, ("Invalid uniform: " + Name \
			+ "\n" + "in shader: " + Path).c_str());*/ \
		if (id != -1) \
		{ \
			func; \
			return true; \
		} \
		return false; \


	bool ShaderProgramOpenGL::SetUniform(const std::string& Name, int Value) const
	{
		_SET_UNIFORM(SetUniform(id, Value));
	}

	bool ShaderProgramOpenGL::SetUniform(const std::string& Name, float Value) const
	{
		_SET_UNIFORM(SetUniform(id, Value));
	}

	bool ShaderProgramOpenGL::SetUniform(const std::string& Name, const Vector2& Value) const
	{
		_SET_UNIFORM(SetUniform(id, Value));
	}

	bool ShaderProgramOpenGL::SetUniform(const std::string& Name, const Vector3& Value) const
	{
		_SET_UNIFORM(SetUniform(id, Value));
	}

	bool ShaderProgramOpenGL::SetUniform(const std::string& Name, const Vector4& Value) const
	{
		_SET_UNIFORM(SetUniform(id, Value));
	}

	bool ShaderProgramOpenGL::SetUniform(const std::string& Name, uint32 Count, const float* Value) const
	{
		_SET_UNIFORM(SetUniform(id, Count, Value));
	}

	bool ShaderProgramOpenGL::SetUniform(const std::string& Name, uint32 Count, const Vector2* Value) const
	{
		_SET_UNIFORM(SetUniform(id, Count, Value));
	}

	bool ShaderProgramOpenGL::SetUniform(const std::string& Name, uint32 Count, const Vector3* Value) const
	{
		_SET_UNIFORM(SetUniform(id, Count, Value));
	}

	bool ShaderProgramOpenGL::SetUniform(const std::string& Name, uint32 Count, const Vector4* Value) const
	{
		_SET_UNIFORM(SetUniform(id, Count, Value));
	}

	bool ShaderProgramOpenGL::SetUniform(const std::string& Name, bool Transpose, const Matrix& Mat) const
	{
		_SET_UNIFORM(SetUniform(id, Transpose, Mat));
	}

	bool ShaderProgramOpenGL::SetUniform(const std::string& Name, Texture* Tex, uint32 Sampler) const
	{
		_SET_UNIFORM(SetUniform(id, static_cast<TextureOpenGL*>(Tex), Sampler));
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
		if (Tex != nullptr)
		{
			Tex->Bind();
		}
		else
		{
			glBindTexture(GL_TEXTURE_2D, 0);
			glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
		}
	}

	ShaderProgramOpenGL::~ShaderProgramOpenGL()
	{
		glDeleteProgram(ID);
	}

}


