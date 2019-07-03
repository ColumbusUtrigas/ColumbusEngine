#include <Graphics/ParticlesRenderer.h>
#include <Graphics/OpenGL/TextureOpenGL.h>
#include <Graphics/OpenGL/ShaderOpenGL.h>
#include <Graphics/Device.h>
#include <GL/glew.h>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>

namespace Columbus
{

	static float Vertices[18] =
	{
		+1, +1, 0,
		-1, +1, 0,
		-1, -1, 0,
		-1, -1, 0,
		+1, -1, 0,
		+1, +1, 0
	};

	static float Texcoords[12] =
	{
		1, 1,
		0, 1,
		0, 0,
		0, 0,
		1, 0,
		1, 1
	};

	static GLuint Particles_VAO = 0;

	void ParticlesRenderer::Allocate(size_t NewSize)
	{
		MaxSize = NewSize;

		delete[] Data;
		Data = new char[MaxSize * sizeof(Vector4) * 6];

		auto Write = BufferUsage::Write;
		auto Static = BufferCpuAccess::Static;
		auto Dynamic = BufferCpuAccess::Dynamic;

		size_t VerticesCount = MaxSize * 6;

		 VerticesBuffer.CreateArray(BufferDesc(MaxSize * sizeof(Vertices),  Write, Static));
		TexcoordsBuffer.CreateArray(BufferDesc(MaxSize * sizeof(Texcoords), Write, Static));
		PositionsBuffer.CreateArray(BufferDesc(VerticesCount * sizeof(Vector3), Write, Dynamic));
		    SizesBuffer.CreateArray(BufferDesc(VerticesCount * sizeof(Vector3), Write, Dynamic));
		   ColorsBuffer.CreateArray(BufferDesc(VerticesCount * sizeof(Vector4), Write, Dynamic));
		OtherDataBuffer.CreateArray(BufferDesc(VerticesCount * sizeof(Vector2), Write, Dynamic));

		for (size_t i = 0; i < MaxSize; i++) memcpy(Data + i * sizeof(Vertices), Vertices, sizeof(Vertices));
		VerticesBuffer.Load(Data);
		for (size_t i = 0; i < MaxSize; i++) memcpy(Data + i * sizeof(Texcoords), Texcoords, sizeof(Texcoords));
		TexcoordsBuffer.Load(Data);

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
			glm::quat Q(glm::vec3(Math::Radians(-MainCamera.Rot.X), Math::Radians(MainCamera.Rot.Y), 0));
			glm::mat4 M = glm::mat4_cast(Q);

			Matrix Billboard;
			memcpy(&Billboard.M[0][0], glm::value_ptr(M), 16 * sizeof(float));

			Shader->SetUniform(Shader->GetFastUniform("View"), false, MainCamera.GetViewMatrix());
			Shader->SetUniform(Shader->GetFastUniform("Projection"), false, MainCamera.GetProjectionMatrix());
			Shader->SetUniform(Shader->GetFastUniform("Billboard"), false, Billboard);
			Shader->SetUniform(Shader->GetFastUniform("Frame"), iVector2(Particles.ModuleSubUV.Horizontal, Particles.ModuleSubUV.Vertical));

			if (Mat.AlbedoMap == nullptr)
				Shader->SetUniform(Shader->GetFastUniform("Texture"), static_cast<TextureOpenGL*>(gDevice->GetDefaultTextures()->White), 0);
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
				for (int a = 0; a < 6; a++)
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

			 VerticesBuffer.VertexAttribute<float>(0, 3, false, 0, 0);
			TexcoordsBuffer.VertexAttribute<float>(1, 2, false, 0, 0);
			PositionsBuffer.VertexAttribute<float>(2, 3, false, 0, 0);
			    SizesBuffer.VertexAttribute<float>(3, 3, false, 0, 0);
			   ColorsBuffer.VertexAttribute<float>(4, 4, false, 0, 0);
			OtherDataBuffer.VertexAttribute<float>(5, 2, false, 0, 0);

			glDrawArrays(GL_TRIANGLES, 0, Particles.Particles.Count * 6);
			glBindVertexArray(0);
		}

		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}

	ParticlesRenderer::~ParticlesRenderer()
	{		
		delete[] Data;

		glDeleteVertexArrays(1, &Particles_VAO);
	}

}


