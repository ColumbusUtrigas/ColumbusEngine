#include <Graphics/ParticlesRenderer.h>
#include <Graphics/OpenGL/TextureOpenGL.h>
#include <Graphics/OpenGL/ShaderOpenGL.h>
#include <Graphics/Device.h>
#include <Math/Quaternion.h>
#include <GL/glew.h>

namespace Columbus
{

	static GLuint Particles_VAO = 0;

	void ParticlesRenderer::Allocate(size_t NewSize)
	{
		MaxSize = NewSize;

		auto Write = BufferUsage::Write;
		auto Dynamic = BufferCpuAccess::Dynamic;

		size_t VerticesCount = MaxSize * 6;

		PositionsBuffer.CreateArray(BufferDesc(VerticesCount * sizeof(Vector3), Write, Dynamic));
		    SizesBuffer.CreateArray(BufferDesc(VerticesCount * sizeof(Vector3), Write, Dynamic));
		   ColorsBuffer.CreateArray(BufferDesc(VerticesCount * sizeof(Vector4), Write, Dynamic));
		OtherDataBuffer.CreateArray(BufferDesc(VerticesCount * sizeof(Vector2), Write, Dynamic));

		PositionsBuffer.Load(nullptr);
		    SizesBuffer.Load(nullptr);
		   ColorsBuffer.Load(nullptr);
		OtherDataBuffer.Load(nullptr);
	}

	ParticlesRenderer::ParticlesRenderer(size_t MaxSize)
	{
		Allocate(MaxSize);

		glGenVertexArrays(1, &Particles_VAO);
		glBindVertexArray(Particles_VAO);
		glBindVertexArray(0);
	}

	void ParticlesRenderer::Render(const ParticleEmitterCPU& Particles, const Camera& MainCamera, const Material& Mat)
	{
		if (MaxSize < Particles.MaxParticles)
		{
			Allocate(Particles.MaxParticles);
		}

		switch (Particles.Blend)
		{
		case ParticleEmitterCPU::BlendMode::Default: // src_alpha * src + (1 - src_alpha) * dst
			glBlendEquation(GL_FUNC_ADD);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			break;
		case ParticleEmitterCPU::BlendMode::Add: // (src_alpha) * src + 1 * dst
			glBlendEquation(GL_FUNC_ADD);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE);
			break;
		case ParticleEmitterCPU::BlendMode::Subtract: break; // TODO
		case ParticleEmitterCPU::BlendMode::Multiply: break; // TODO
		}

		glDisable(GL_CULL_FACE);
		glDepthMask(GL_FALSE);

		ShaderProgramOpenGL* Shader = static_cast<ShaderProgramOpenGL*>(Mat.GetShader());
		if (Shader != nullptr)
		{
			Quaternion Q(Vector3(Math::Radians(-MainCamera.Rot.X), Math::Radians(MainCamera.Rot.Y), 0));
			Matrix Billboard = Q.ToMatrix();

			Shader->SetUniform(Shader->GetFastUniform("View"), false, MainCamera.GetViewMatrix());
			Shader->SetUniform(Shader->GetFastUniform("Projection"), false, MainCamera.GetProjectionMatrix());
			Shader->SetUniform(Shader->GetFastUniform("Billboard"), false, Billboard);
			Shader->SetUniform(Shader->GetFastUniform("Frame"), iVector2(Particles.ModuleSubUV.Horizontal, Particles.ModuleSubUV.Vertical));

			if (Mat.AlbedoMap == nullptr)
				Shader->SetUniform(Shader->GetFastUniform("Texture"), static_cast<TextureOpenGL*>(gDevice->GetDefaultTextures()->White.get()), 0);
			else
				Shader->SetUniform(Shader->GetFastUniform("Texture"), static_cast<TextureOpenGL*>(Mat.AlbedoMap), 0);

			Vector3* Positions;
			Vector3* Sizes;
			Vector4* Colors;
			Vector2* OtherData;

			PositionsBuffer.Map((void*&)Positions, BufferMapAccess::Write);
			    SizesBuffer.Map((void*&)Sizes,     BufferMapAccess::Write);
			   ColorsBuffer.Map((void*&)Colors,    BufferMapAccess::Write);
			OtherDataBuffer.Map((void*&)OtherData, BufferMapAccess::Write);

			for (size_t i = 0; i < Particles.Particles.Count; i++)
			{
				for (size_t a = 0; a < 6; a++)
				{
					Positions[i * 6 + a]   = Particles.Particles.Positions[i].XYZ();
					    Sizes[i * 6 + a]   = Particles.Particles.Sizes[i];
					   Colors[i * 6 + a]   = Particles.Particles.Colors[i];
					OtherData[i * 6 + a].X = Particles.Particles.Rotations[i];
					OtherData[i * 6 + a].Y = Particles.Particles.Frames[i];
				}
			}

			PositionsBuffer.Unmap();
			    SizesBuffer.Unmap();
			   ColorsBuffer.Unmap();
			OtherDataBuffer.Unmap();

			glBindVertexArray(Particles_VAO);

			PositionsBuffer.VertexAttribute<float>(0, 3, false, 0, 0);
			    SizesBuffer.VertexAttribute<float>(1, 3, false, 0, 0);
			   ColorsBuffer.VertexAttribute<float>(2, 4, false, 0, 0);
			OtherDataBuffer.VertexAttribute<float>(3, 2, false, 0, 0);

			glDrawArrays(GL_TRIANGLES, 0, Particles.Particles.Count * 6);
			glBindVertexArray(0);
		}

		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}

	ParticlesRenderer::~ParticlesRenderer()
	{		
		glDeleteVertexArrays(1, &Particles_VAO);
	}

}


