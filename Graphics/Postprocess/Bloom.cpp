#include <Graphics/Postprocess/Bloom.h>
#include <Graphics/OpenGL/ShaderOpenGL.h>
#include <Graphics/OpenGL/TextureOpenGL.h>
#include <Profiling/Profiling.h>
#include <Graphics/Postprocess/GaussBlur.h>

namespace Columbus
{

	void PostprocessBloom::Draw(Texture*& Frame, const iVector2& FrameSize,
		std::array<PostEffect, 2>& aPost)
	{
		static std::array<PostEffect, 2> Post;
		static bool first = true;

		if (first)
		{
			Post[0].ColorTexturesEnablement[0] = true;
			Post[1].ColorTexturesEnablement[0] = true;
			Post[0].ColorTexturesFormats[0] = TextureFormat::R11G11B10F;
			Post[1].ColorTexturesFormats[0] = TextureFormat::R11G11B10F;
			Post[0].ColorTextureFlags[0] = Texture::Flags(Texture::Filter::Trilinear, Texture::Anisotropy::Anisotropy1, Texture::Wrap::Clamp);
			Post[1].ColorTextureFlags[0] = Texture::Flags(Texture::Filter::Trilinear, Texture::Anisotropy::Anisotropy1, Texture::Wrap::Clamp);
			Post[0].ColorTexturesMipmaps[0] = true;
			Post[1].ColorTexturesMipmaps[0] = true;

			first = false;
		}

		if (Enabled)
		{
			PROFILE_GPU(ProfileModuleGPU::BloomStage);
			
			auto brightShader = (ShaderProgramOpenGL*)gDevice->GetDefaultShaders()->BloomBright.get();
			auto blurShader = (ShaderProgramOpenGL*)gDevice->GetDefaultShaders()->GaussBlur.get();
			auto bloomShader = (ShaderProgramOpenGL*)gDevice->GetDefaultShaders()->Bloom.get();

			// bloom bright pass
			Post[0].ColorTexturesEnablement[0] = true;
			Post[0].Bind({}, {0}, FrameSize);
			//Post[0].Mipmaps();

			brightShader->Bind();
			brightShader->SetUniform("BaseTexture", (TextureOpenGL*)Frame, 0);
			brightShader->SetUniform("Treshold", Treshold);
			_Quad.Render();

			iVector2 resolution;

			switch (Resolution)
			{
			case BloomResolutionType::Quad: resolution = FrameSize / 2; break;
			case BloomResolutionType::Half: resolution = iVector2(Vector2(FrameSize) / sqrtf(2)); break;
			case BloomResolutionType::Full: resolution = FrameSize; break;
			}

			if (resolution.X == 0) resolution.X = 1;
			if (resolution.Y == 0) resolution.Y = 1;
			
			// bloom blur pass
			/*blurShader->Bind();
			blurShader->SetUniform("Radius", Radius);

			Post[1].Bind({}, { 0 }, resolution);
			Post[1].Mipmaps();

			for (int i = 0; i < Iterations; i++)
			{
				glViewport(0, 0, resolution.X >> i, resolution.Y >> i);
				Post[1].FB->SetTexture2D(Framebuffer::Attachment::Color0, Post[1].ColorTextures[0], i);
				if (i == 0)
					blurShader->SetUniform("BaseTexture", Post[0].ColorTextures[0], 0);
				else
					blurShader->SetUniform("BaseTexture", Post[1].ColorTextures[0], 0);

				blurShader->SetUniform("Radius", Math::Max(float(i-1), 0.0f));
				blurShader->SetUniform("Horizontal", 0);
				_Quad.Render();
			}

			Post[0].Bind({1}, { 0 }, resolution);

			for (int i = 0; i < Iterations; i++)
			{
				glViewport(0, 0, resolution.X >> i, resolution.Y >> i);
				Post[0].FB->SetTexture2D(Framebuffer::Attachment::Color0, Post[0].ColorTextures[0], i);
				if (i == 0)
					blurShader->SetUniform("BaseTexture", Post[1].ColorTextures[0], 0);
				else
					blurShader->SetUniform("BaseTexture", Post[0].ColorTextures[0], 0);

				blurShader->SetUniform("Radius", Math::Max(float(i - 1), 0.0f));
				blurShader->SetUniform("Horizontal", 1);
				_Quad.Render();
			}*/

			/*auto ss = static_cast<ShaderProgramOpenGL*>(gDevice->GetDefaultShaders()->ScreenSpace.get());
			Post[0].ColorTexturesEnablement[0] = true;
			Post[0].ColorTexturesMipmaps[0] = true;
			Post[0].Bind({}, { 0 }, resolution);
			ss->Bind();
			ss->SetUniform("BaseTexture", Post[1].ColorTextures[0], 0);
			_Quad.Render();*/
			

			/*for (int i = 0; i < Iterations; i++)
			{			
				Post[1].ColorTexturesEnablement[0] = true;
				Post[1].Bind({}, { 0 }, resolution);

				blurShader->SetUniform("BaseTexture", (TextureOpenGL*)Post[0].ColorTextures[0], 0);
				blurShader->SetUniform("Horizontal", 1);
				_Quad.Render();

				Post[0].ColorTexturesEnablement[0] = true;
				Post[0].Bind({}, { 0 }, resolution);

				blurShader->SetUniform("BaseTexture", (TextureOpenGL*)Post[1].ColorTextures[0], 0);
				blurShader->SetUniform("Horizontal", 0);
				_Quad.Render();
			}*/

			// bloom final pass
			auto blur = GaussBlur(Post[0].ColorTextures[0], resolution, Iterations);
			Post[1].ColorTexturesEnablement[0] = true;
			Post[1].Bind({}, {}, FrameSize);

			bloomShader->Bind();
			bloomShader->SetUniform("BaseTexture", (TextureOpenGL*)Frame, 0);
			bloomShader->SetUniform("Blur", blur, 1);
			bloomShader->SetUniform("Intensity", Intensity);
			bloomShader->SetUniform("Radius", Radius);
			_Quad.Render();

			// output
			Post[1].ColorTextures[0]->SetMipmapLevel(0, 0);
			Frame = Post[1].ColorTextures[0];
		}
	}

}


