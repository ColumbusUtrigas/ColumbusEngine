#include <Graphics/ShaderBuilder.h>
#include <cstring>

namespace Columbus
{

	const char* gCommonShaderHeader = 
	R"(
	#version 130
	#define Texture2D sampler2D
	#define Texture3D sampler3D
	#define TextureCube samplerCube
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
	#endif
	)";

	const char* gVertexShaderHeader = 
	R"(
	#define Position gl_Position
	#define VertexShader

	)";

	const char* gFragmentShaderHeader = 
	R"(
	#define FragData gl_FragData
	#define FragmentShader
	out vec4 FragColor;

	)";

	// Nya ^_^. Yefim is the best! Vanya is cool too (but Yefim better). Julia, I love you
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


