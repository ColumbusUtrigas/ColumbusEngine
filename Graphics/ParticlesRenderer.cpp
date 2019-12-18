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

		auto Array = BufferType::Array;
		auto Write = BufferUsage::Write;
		auto Dynamic = BufferCpuAccess::Dynamic;

		size_t VerticesCount = MaxSize * 6;

		gDevice->CreateBuffer(BufferDesc(VerticesCount * sizeof(Vector3), Array, Write, Dynamic), &PositionsBuffer);
		gDevice->CreateBuffer(BufferDesc(VerticesCount * sizeof(Vector3), Array, Write, Dynamic), &SizesBuffer);
		gDevice->CreateBuffer(BufferDesc(VerticesCount * sizeof(Vector4), Array, Write, Dynamic), &ColorsBuffer);
		gDevice->CreateBuffer(BufferDesc(VerticesCount * sizeof(Vector2), Array, Write, Dynamic), &OtherDataBuffer);
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

		DepthStencilStateDesc DSDesc;
		DSDesc.DepthEnable = true;
		DSDesc.DepthWriteMask = false;
		DSDesc.StencilEnable = false;

		DepthStencilState DSState;
		DSState.Desc = DSDesc;

		gDevice->OMSetDepthStencilState(&DSState, 0);

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

			gDevice->MapBuffer(PositionsBuffer, BufferMapAccess::Write, (void*&)Positions);
			gDevice->MapBuffer(SizesBuffer,     BufferMapAccess::Write, (void*&)Sizes);
			gDevice->MapBuffer(ColorsBuffer,    BufferMapAccess::Write, (void*&)Colors);
			gDevice->MapBuffer(OtherDataBuffer, BufferMapAccess::Write, (void*&)OtherData);

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

			gDevice->UnmapBuffer(PositionsBuffer);
			gDevice->UnmapBuffer(SizesBuffer);
			gDevice->UnmapBuffer(ColorsBuffer);
			gDevice->UnmapBuffer(OtherDataBuffer);

			glBindVertexArray(Particles_VAO);

			InputLayout layout;
			layout.NumElements = 4;
			layout.Elements[0] = InputLayoutElementDesc{ 0, 3 };
			layout.Elements[1] = InputLayoutElementDesc{ 1, 3 };
			layout.Elements[2] = InputLayoutElementDesc{ 2, 4 };
			layout.Elements[3] = InputLayoutElementDesc{ 3, 2 };

			Buffer* ppBuffers[] = { PositionsBuffer, SizesBuffer, ColorsBuffer, OtherDataBuffer };
			gDevice->IASetInputLayout(&layout);
			gDevice->IASetVertexBuffers(0, 4, ppBuffers);
			gDevice->IASetPrimitiveTopology(PrimitiveTopology::TriangleList);
			gDevice->Draw(Particles.Particles.Count * 6, 0);

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


