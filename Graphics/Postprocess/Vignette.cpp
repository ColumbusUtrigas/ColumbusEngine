#include <Graphics/Postprocess/Vignette.h>
#include <Graphics/Device.h>

#include <Graphics/OpenGL/ShaderOpenGL.h>

namespace Columbus
{

	void PostprocessVignette::Draw(const BufferOpenGL& UBO, size_t Offset, size_t Size)
	{
		auto shader = (ShaderProgramOpenGL*)gDevice->GetDefaultShaders()->Vignette;

		// 0 is binding in the shader
		UBO.BindRange(0, Offset, Size);
		shader->Bind();
		_Quad.Render();
		shader->Unbind();
	}

}


