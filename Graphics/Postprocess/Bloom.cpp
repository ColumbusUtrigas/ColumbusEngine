#include <Graphics/Postprocess/Bloom.h>
#include <Graphics/OpenGL/ShaderOpenGL.h>
#include <Graphics/OpenGL/TextureOpenGL.h>
#include <Profiling/Profiling.h>

namespace Columbus
{

	void PostprocessBloom::Draw(Texture*& Frame, const iVector2& FrameSize,
		std::array<PostEffect, 2>& Post)
	{
		if (Enabled)
		{
			PROFILE_GPU(ProfileModuleGPU::BloomStage);
			
			auto brightShader = (ShaderProgramOpenGL*)gDevice->GetDefaultShaders()->BloomBright.get();
			auto blurShader = (ShaderProgramOpenGL*)gDevice->GetDefaultShaders()->GaussBlur.get();
			auto bloomShader = (ShaderProgramOpenGL*)gDevice->GetDefaultShaders()->Bloom.get();

			// bloom bright pass
			Post[0].ColorTexturesEnablement[0] = true;
			Post[0].Bind({}, {0}, FrameSize);

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
			blurShader->Bind();
			blurShader->SetUniform("Radius", Radius);

			for (int i = 0; i < Iterations; i++)
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
			}

			// bloom final pass
			Post[1].ColorTexturesEnablement[0] = true;
			Post[1].Bind({}, {}, FrameSize);

			bloomShader->Bind();
			bloomShader->SetUniform("BaseTexture", (TextureOpenGL*)Frame, 0);
			bloomShader->SetUniform("Blur", (TextureOpenGL*)Post[0].ColorTextures[0], 1);
			bloomShader->SetUniform("Intensity", Intensity);
			_Quad.Render();

			// output
			Frame = Post[1].ColorTextures[0];
		}
	}

}


