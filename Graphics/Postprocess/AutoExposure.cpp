#include <Graphics/Postprocess/AutoExposure.h>

#include <Graphics/OpenGL/ShaderOpenGL.h>
#include <Graphics/OpenGL/TextureOpenGL.h>

namespace Columbus
{

	PostprocessAutoExposure::PostprocessAutoExposure(ScreenQuad& Quad) :
		_Quad(Quad)
	{
		if (gDevice == nullptr) return;

		for (auto& buf : _Bufs)
		{
			buf.ColorTexturesEnablement[0] = true;
			buf.ColorTexturesFormats[0] = TextureFormat::RGBA32F;
			buf.Bind({0}, {0}, {1});
			buf.Unbind();
		}
	}

	Texture* PostprocessAutoExposure::Draw(float Exposure, Texture* Frame, iVector2 FrameSize,
			Buffer* UBO, size_t Offset, size_t Size)
	{
		if (Enabled)
		{
			auto prev = (_CurrentBuf + (_BufSize - 1)) % _BufSize;
			auto shader = (ShaderProgramOpenGL*)gDevice->GetDefaultShaders()->AutoExposure.get();

			if (_FirstDraw)
			{
				// fill previous buffer with Exposure
				_Bufs[prev].Bind({Exposure}, {0}, {1});
				_FirstDraw = false;
			}

			int lastMip = floor(log2(Math::Max(FrameSize.X, FrameSize.Y)));
			Frame->SetMipmapLevel(lastMip, 1000);

			_Bufs[_CurrentBuf].Bind({0}, {0}, {1});
			shader->Bind();
			shader->SetUniform("BaseTexture", (TextureOpenGL*)Frame, 0);
			shader->SetUniform("Previous", (TextureOpenGL*)_Bufs[prev].ColorTextures[0], 1);
			gDevice->BindBufferRange(UBO, 0, Offset, Size);
			_Quad.Render();

			Frame->SetMipmapLevel(0, 1000);
			_CurrentBuf = (_CurrentBuf + 1) % _BufSize;
		} else
		{
			_FirstDraw = true;
		}

		return _Bufs[_CurrentBuf].ColorTextures[0];
	}

}


