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
			
			auto brightShader = (ShaderProgramOpenGL*)gDevice->GetDefaultShaders()->BloomBright;
			auto blurShader = (ShaderProgramOpenGL*)gDevice->GetDefaultShaders()->GaussBlur;
			auto bloomShader = (ShaderProgramOpenGL*)gDevice->GetDefaultShaders()->Bloom;

			static int brightParamTexture = brightShader->GetFastUniform("BaseTexture");
			static int brightParamTreshold = brightShader->GetFastUniform("Treshold");

			static int blurParamTexture = blurShader->GetFastUniform("BaseTexture");
			static int blurParamHorizontal = blurShader->GetFastUniform("Horizontal");
			static int blurParamRadius = blurShader->GetFastUniform("Radius");

			static int bloomParamTexture = bloomShader->GetFastUniform("BaseTexture");
			static int bloomParamBlur = bloomShader->GetFastUniform("Blur");
			static int bloomParamIntensity = bloomShader->GetFastUniform("Intensity");

			// bloom bright pass
			Post[0].ColorTexturesEnablement[0] = true;
			Post[0].Bind({}, {0}, FrameSize);

			brightShader->Bind();
			brightShader->SetUniform(brightParamTexture, (TextureOpenGL*)Frame, 0);
			brightShader->SetUniform(brightParamTreshold, Treshold);
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
			blurShader->SetUniform(blurParamRadius, Radius);

			for (int i = 0; i < Iterations; i++)
			{			
				Post[1].ColorTexturesEnablement[0] = true;
				Post[1].Bind({}, { 0 }, resolution);

				blurShader->SetUniform(blurParamTexture, (TextureOpenGL*)Post[0].ColorTextures[0], 0);
				blurShader->SetUniform(blurParamHorizontal, 1);
				_Quad.Render();

				Post[0].ColorTexturesEnablement[0] = true;
				Post[0].Bind({}, { 0 }, resolution);

				blurShader->SetUniform(blurParamTexture, (TextureOpenGL*)Post[1].ColorTextures[0], 0);
				blurShader->SetUniform(blurParamHorizontal, 0);
				_Quad.Render();
			}

			// bloom final pass
			Post[1].ColorTexturesEnablement[0] = true;
			Post[1].Bind({}, {}, FrameSize);

			bloomShader->Bind();
			bloomShader->SetUniform(bloomParamTexture, (TextureOpenGL*)Frame, 0);
			bloomShader->SetUniform(bloomParamBlur, (TextureOpenGL*)Post[0].ColorTextures[0], 1);
			bloomShader->SetUniform(bloomParamIntensity, Intensity);
			_Quad.Render();

			// output
			Frame = Post[1].ColorTextures[0];
		}
	}

}


