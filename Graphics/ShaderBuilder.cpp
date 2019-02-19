#include <Graphics/ShaderBuilder.h>
#include <cstring>

namespace Columbus
{

	const char* gCommonShaderHeader =
	"#version 130\n"
	"#define Texture2D sampler2D\n"
	"#define Texture3D sampler3D\n"
	"#define TextureCube samplerCube\n"
	"#if __VERSION__ < 130\n"
		"#define Sample2D(tex, uv) texture2D(tex, uv)\n"
		"#define Sample3D(tex, uv) texture3D(tex, uv)\n"
		"#define SampleCube(tex, uv) textureCube(tex, uv)\n"

		"#define Sample2DLod(tex, uv, lod) texture2DLod(tex, uv, lod)\n"
		"#define Sample3DLod(tex, uv, lod) texture3DLod(tex, uv, lod)\n"
		"#define SampleCubeLod(tex, uv, lod) textureCubeLod(tex, uv, lod)\n"
	"#else\n"
		"#define Sample2D(tex, uv) texture(tex, uv)\n"
		"#define Sample3D(tex, uv) texture(tex, uv)\n"
		"#define SampleCube(tex, uv) texture(tex, uv)\n"

		"#define Sample2DLod(tex, uv, lod) textureLod(tex, uv, lod)\n"
		"#define Sample3DLod(tex, uv, lod) textureLod(tex, uv, lod)\n"
		"#define SampleCubeLod(tex, uv, lod) textureLod(tex, uv, lod)\n"
	"#endif\n\n";

	const char* gVertexShaderHeader =
	"#define Position gl_Position\n"
	"#define VertexShader\n\n";
	
	const char* gFragmentShaderHeader =
	"#define FragData gl_FragData\n"
	"#define FragmentShader\n"
	"out vec4 FragColor;\n\n";

	// Nya ^_^
	int Cat(char*& ResultShader, const char* Header, const char* ShaderSource, int Length, int HeaderLength)
	{
		int ShaderSourceLength = HeaderLength + Length + 1;
		ResultShader = new char[ShaderSourceLength + 1];
		memset(ResultShader, 0, ShaderSourceLength + 1);

		strcat(ResultShader, gCommonShaderHeader);
		strcat(ResultShader, Header);
		strcat(ResultShader, ShaderSource);
		strcat(ResultShader, "\n");
		return ShaderSourceLength;
	}
	
	bool ShaderBuilder::Build(const char* InShader, ShaderType Type)
	{
		delete[] ShaderSource;
		ShaderSourceLength = 0;

		static int VertexHeaderLength = strlen(gCommonShaderHeader) + strlen(gVertexShaderHeader);
		static int FragmentHeaderLength = strlen(gCommonShaderHeader) + strlen(gFragmentShaderHeader);

		int InputLength = strlen(InShader);

		switch (Type)
		{
		case ShaderType::Vertex:
		{
			ShaderSourceLength = Cat(ShaderSource, gVertexShaderHeader, InShader, InputLength, VertexHeaderLength);
			return true;
			break;
		}

		case ShaderType::Fragment:
			ShaderSourceLength = Cat(ShaderSource, gFragmentShaderHeader, InShader, InputLength, FragmentHeaderLength);
			return true;
			break;
		};

		return false;
	}

}


