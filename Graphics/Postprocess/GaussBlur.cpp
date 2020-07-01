#include <Graphics/Postprocess/GaussBlur.h>
#include <Graphics/OpenGL/ShaderOpenGL.h>
#include <Graphics/ScreenQuad.h>

namespace Columbus
{

	Texture* GaussBlur(Texture* base, iVector2 ContextSize, int mips)
	{
		auto Prefilter = static_cast<ShaderProgramOpenGL*>(gDevice->GetDefaultShaders()->GaussBlur.get());
		static bool first = true;
		static Framebuffer* prefilterFB[2];
		static Texture* PrefilterTex[2];
		static iVector2 PrevSize(0);
		static ScreenQuad _Quad;
		if (first)
		{

			prefilterFB[0] = gDevice->CreateFramebuffer();
			prefilterFB[1] = gDevice->CreateFramebuffer();
			PrefilterTex[0] = gDevice->CreateTexture();
			PrefilterTex[1] = gDevice->CreateTexture();
			first = false;
		}

		if (ContextSize != PrevSize)
		{
			Texture::Flags F;
			F.Filtering = Texture::Filter::Trilinear;
			F.Wrapping = Texture::Wrap::Clamp;
			F.AnisotropyFilter = Texture::Anisotropy::Anisotropy1;

			PrefilterTex[0]->Create2D(TextureDesc(ContextSize.X, ContextSize.Y, 0, 0, TextureFormat::R11G11B10F));
			PrefilterTex[0]->GenerateMipmap();
			PrefilterTex[0]->SetFlags(F);

			PrefilterTex[1]->Create2D(TextureDesc(ContextSize.X, ContextSize.Y, 0, 0, TextureFormat::R11G11B10F));
			PrefilterTex[1]->GenerateMipmap();
			PrefilterTex[1]->SetFlags(F);
			PrevSize = ContextSize;
		}

		base->SetMipmapLevel(0, 0);

		glDepthMask(GL_FALSE);
		for (int j = 0; j < 2; j++)
		{
			prefilterFB[j]->Bind();
			for (int i = 0; i < mips; i++)
			{
				prefilterFB[j]->SetTexture2D(Framebuffer::Attachment::Color0, PrefilterTex[j], i);
				glViewport(0, 0, ContextSize.X >> i, ContextSize.Y >> i);

				if (i == 0)
				{
					auto ss = static_cast<ShaderProgramOpenGL*>(gDevice->GetDefaultShaders()->ScreenSpace.get());
					ss->Bind();
					ss->SetUniform("BaseTexture", base, 0);

					_Quad.Render();
				}
				else
				{
					Prefilter->Bind();
					Prefilter->SetUniform("BaseTexture", PrefilterTex[0], 0);
					Prefilter->SetUniform("Horizontal", 1 - j);
					Prefilter->SetUniform("Radius", float(i - 1));

					_Quad.Render();
				}
			}

			base = PrefilterTex[j];
		}
		glDepthMask(GL_TRUE);

		return PrefilterTex[1];
	}

}
