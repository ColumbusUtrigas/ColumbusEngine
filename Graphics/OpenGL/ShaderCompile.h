#pragma once

#include <Graphics/Types.h>
#include <Core/SmartPointer.h>
#include <Core/Assert.h>
#include <string_view>

namespace Columbus::Graphics::GL
{

	GLuint CompileShaderStage_GL(SPtr<ShaderStage> stage)
	{
		if (!stage)
			return 0;

		GLint len = stage->Source.size();;
		const GLchar* str = stage->Source.data();

		GLenum gltype = 0;
		const char* strtype = "";

		switch (stage->Type)
		{
		case ShaderType::Vertex:
			gltype = GL_VERTEX_SHADER;
			strtype = "Vertex";
			break;
		case ShaderType::Pixel:
			gltype = GL_FRAGMENT_SHADER;
			strtype = "Pixel";
			break;
		case ShaderType::Hull:
			gltype = GL_TESS_CONTROL_SHADER;
			strtype = "Hull";
			break;
		case ShaderType::Domain:
			gltype = GL_TESS_EVALUATION_SHADER;
			strtype = "Domain";
			break;
		case ShaderType::Geometry:
			gltype = GL_GEOMETRY_SHADER;
			strtype = "Geometry";
			break;
		case ShaderType::Compute:
			gltype = GL_COMPUTE_SHADER;
			strtype = "Compute";
			break;
		}

		auto shadid = glCreateShader(gltype);
		glShaderSource(shadid, 1, &str, &len);
		glCompileShader(shadid);

		int32 Status = GL_TRUE;
		int32 Length = 0;
		char* Error = nullptr;

		glGetShaderiv(shadid, GL_COMPILE_STATUS, &Status);
		glGetShaderiv(shadid, GL_INFO_LOG_LENGTH, &Length);
		Error = new char[Length];
		glGetShaderInfoLog(shadid, Length, &Length, Error);

		if (Status == GL_FALSE)
		{
			Log::Error("%s shader (%s): %s", strtype, "IDK", Error);
		}
		else if (Length > 1)
		{
			Log::Warning("%s shader (%s): %s", strtype, "IDK", Error);
		}

		return shadid;
	}

}
