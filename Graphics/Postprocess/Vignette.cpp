#include <Graphics/Postprocess/Vignette.h>
#include <Graphics/Device.h>

namespace Columbus
{

	PostprocessVignette::PostprocessVignette(ScreenQuad& Quad) : _Quad(Quad)
	{
		if (gDevice == nullptr) return;

		BlendStateDesc BSD;
		BSD.RenderTarget[0].BlendEnable = true;
		BSD.RenderTarget[0].SrcBlend = Blend::SrcAlpha;
		BSD.RenderTarget[0].DestBlend = Blend::InvSrcAlpha;
		BSD.RenderTarget[0].SrcBlendAlpha = Blend::SrcAlpha;
		BSD.RenderTarget[0].DestBlendAlpha = Blend::InvSrcAlpha;

		gDevice->CreateBlendState(BSD, &BS);
	}

	void PostprocessVignette::Draw(Buffer* UBO, size_t Offset, size_t Size)
	{
		// 0 is binding in the shader
		gDevice->BindBufferRange(UBO, 0, Offset, Size);
		gDevice->SetShader(gDevice->GetDefaultShaders()->Vignette.get());
		gDevice->OMSetBlendState(BS, nullptr, 0xFFFFFFFF);
		_Quad.Render();
	}

}
