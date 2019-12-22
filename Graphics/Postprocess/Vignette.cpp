#include <Graphics/Postprocess/Vignette.h>
#include <Graphics/Device.h>

#include <Graphics/OpenGL/ShaderOpenGL.h>

namespace Columbus
{

	void PostprocessVignette::Draw(Buffer* UBO, size_t Offset, size_t Size)
	{
		auto shader = static_cast<ShaderProgramOpenGL*>(gDevice->GetDefaultShaders()->Vignette.get());

		// 0 is binding in the shader
		gDevice->BindBufferRange(UBO, 0, Offset, Size);
		gDevice->SetShader(gDevice->GetDefaultShaders()->Vignette.get());
		_Quad.Render();
	}

}


