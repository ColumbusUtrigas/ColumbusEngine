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

	void ParticlesRenderer::Allocate(size_t NewSize)
	{
		MaxSize = NewSize;

		delete[] Data;
		Data = new char[MaxSize * sizeof(Vector4) * 6];

		auto Write = BufferOpenGL::UsageType::Write;
		auto Static = BufferOpenGL::FrequencyType::Static;
		auto Dynamic = BufferOpenGL::FrequencyType::Dynamic;

		size_t VerticesCount = MaxSize * 6;

		for (size_t i = 0; i < MaxSize; i++) memcpy(Data + i * sizeof(Vertices), Vertices, sizeof(Vertices));
		VerticesBuffer.Load(Data, BufferOpenGL::Properties(MaxSize * sizeof(Vertices), Write, Static));
		for (size_t i = 0; i < MaxSize; i++) memcpy(Data + i * sizeof(Texcoords), Texcoords, sizeof(Texcoords));
		TexcoordsBuffer.Load(Data, BufferOpenGL::Properties(MaxSize * sizeof(Texcoords), Write, Static));

		PositionsBuffer.Load(nullptr, BufferOpenGL::Properties(VerticesCount * sizeof(Vector3), Write, Dynamic));
		SizesBuffer.Load(nullptr, BufferOpenGL::Properties(VerticesCount * sizeof(Vector3), Write, Dynamic));
		ColorsBuffer.Load(nullptr, BufferOpenGL::Properties(VerticesCount * sizeof(Vector4), Write, Dynamic));
		OtherDataBuffer.Load(nullptr, BufferOpenGL::Properties(VerticesCount * sizeof(Vector2), Write, Dynamic));
	}

	ParticlesRenderer::ParticlesRenderer(size_t MaxSize)
	{
		VerticesBuffer.CreateArray(BufferOpenGL::Properties());
		TexcoordsBuffer.CreateArray(BufferOpenGL::Properties());
		PositionsBuffer.CreateArray(BufferOpenGL::Properties());
		SizesBuffer.CreateArray(BufferOpenGL::Properties());
		ColorsBuffer.CreateArray(BufferOpenGL::Properties());
		OtherDataBuffer.CreateArray(BufferOpenGL::Properties());

		Allocate(MaxSize);
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

			Vector3* Positions = (Vector3*)PositionsBuffer.Map(BufferOpenGL::AccessType::WriteOnly);
			for (size_t i = 0; i < Particles.Particles.Count; i++) for (int a = 0; a < 6; a++) Positions[i * 6 + a] = Particles.Particles.Positions[i].XYZ();
			PositionsBuffer.Unmap();

			Vector3* Sizes = (Vector3*)SizesBuffer.Map(BufferOpenGL::AccessType::WriteOnly);
			for (size_t i = 0; i < Particles.Particles.Count; i++) for (int a = 0; a < 6; a++) Sizes[i * 6 + a] = Particles.Particles.Sizes[i];
			SizesBuffer.Unmap();

			Vector4* Colors = (Vector4*)ColorsBuffer.Map(BufferOpenGL::AccessType::WriteOnly);
			for (size_t i = 0; i < Particles.Particles.Count; i++) for (int a = 0; a < 6; a++) Colors[i * 6 + a] = Particles.Particles.Colors[i];
			ColorsBuffer.Unmap();

			Vector2* OtherData = (Vector2*)OtherDataBuffer.Map(BufferOpenGL::AccessType::WriteOnly);
			for (size_t i = 0; i < Particles.Particles.Count; i++) for (int a = 0; a < 6; a++) OtherData[i * 6 + a] = Vector2(Particles.Particles.Rotations[i], (float)Particles.Particles.Frames[i]);
			OtherDataBuffer.Unmap();

			 VerticesBuffer.VertexAttribute<float>(0, 3, false, 0, nullptr);
			TexcoordsBuffer.VertexAttribute<float>(1, 2, false, 0, nullptr);
			PositionsBuffer.VertexAttribute<float>(2, 3, false, 0, nullptr);
			    SizesBuffer.VertexAttribute<float>(3, 3, false, 0, nullptr);
			   ColorsBuffer.VertexAttribute<float>(4, 4, false, 0, nullptr);
			OtherDataBuffer.VertexAttribute<float>(5, 2, false, 0, nullptr);

			glDrawArrays(GL_TRIANGLES, 0, Particles.Particles.Count * 6);
		}

		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}

	ParticlesRenderer::~ParticlesRenderer()
	{
		delete[] Data;
	}

}


