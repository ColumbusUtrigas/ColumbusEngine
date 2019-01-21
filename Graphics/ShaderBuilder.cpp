#include <Graphics/ShaderBuilder.h>
#include <cstring>

namespace Columbus
{

	const char* gVertexShaderHeader =
	"#version 130\n"
	"#define Position gl_Position\n"
	"#define VertexShader\n";
	
	const char* gFragmentShaderHeader =
	"#version 130\n"
	"#define FragData gl_FragData\n"
	"#define FragmentShader\n"
	"out vec4 FragColor;\n";
	
	bool ShaderBuilder::Build(const char* InShader, ShaderType Type)
	{
		delete[] ShaderSource;
		ShaderSourceLength = 0;

		switch (Type)
		{
		case ShaderType::Vertex:
		{
			ShaderSourceLength = strlen(gVertexShaderHeader) + 1 + strlen(InShader) + 1;
			ShaderSource = new char[ShaderSourceLength + 1];
			memset(ShaderSource, 0, ShaderSourceLength + 1);
			strcat(strcat(strcat(strcat(ShaderSource, gVertexShaderHeader), "\n"), InShader), "\n");
			return true;
			break;
		}

		case ShaderType::Fragment:
			ShaderSourceLength = strlen(gFragmentShaderHeader) + 1 + strlen(InShader) + 1;
			ShaderSource = new char[ShaderSourceLength + 1];
			memset(ShaderSource, 0, ShaderSourceLength + 1);
			strcat(strcat(strcat(strcat(ShaderSource, gFragmentShaderHeader), "\n"), InShader), "\n");
			return true;
			break;
		};

		return false;
	}

}


