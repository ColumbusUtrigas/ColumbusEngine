#include <Impl/ImplOpenGL.h>

namespace C
{

	bool C_GetErrorOpenGL()
	{
		GLenum err;
		if (err != GL_NO_ERROR)
		{
			std::string error;

			switch (err)
			{
			case GL_INVALID_ENUM:           error = "INVALID_ENUM";           break;
			case GL_INVALID_VALUE:          error = "INVALID_VALUE";          break;
			case GL_INVALID_OPERATION:      error = "INVALID_OPERATION";      break;
			case GL_STACK_OVERFLOW:         error = "STACK_OVERFLOW";         break;
			case GL_STACK_UNDERFLOW:        error = "STACK_UNDERFLOW";        break;
			case GL_OUT_OF_MEMORY:          error = "OUT_OF_MEMORY";          break;
			case GL_INVALID_FRAMEBUFFER_OPERATION:  error = "INVALID_FRAMEBUFFER_OPERATION";  break;
			case GL_TABLE_TOO_LARGE:        error = "TABLE_TOO_LARGE";        break;
			}

			C_Error("OpenGL: %s\n", error.c_str());
			return true;
		}
		return false;
	}

}

