#pragma once

#include <Graphics/Buffer.h>
#include <GL/glew.h>

namespace Columbus
{

	class BufferOpenGL : public Buffer
	{
	private:
		GLuint gl_Handle = 0;
	public:
		BufferOpenGL(const BufferDesc& Desc)
		{
			this->Desc = Desc;
		}

		void* GetHandle() final override
		{
			return static_cast<void*>(&gl_Handle);
		}
	};

}
