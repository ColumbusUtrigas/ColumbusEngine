#include <Graphics/ParticlesRenderer.h>
#include <Graphics/OpenGL/TextureOpenGL.h>
#include <Graphics/OpenGL/ShaderOpenGL.h>
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
		char* Data = new char[MaxSize * sizeof(Vector4) * 6];

		for (size_t i = 0; i < MaxSize; i++) memcpy(Data + i * sizeof(Vertices), Vertices, sizeof(Vertices));
		VerticesBuffer.Load(Buffer::Properties{ MaxSize * sizeof(Vertices), Buffer::Usage::Write, Buffer::Changing::Static }, Data);
		for (size_t i = 0; i < MaxSize; i++) memcpy(Data + i * sizeof(Texcoords), Texcoords, sizeof(Texcoords));
		TexcoordsBuffer.Load(Buffer::Properties{ MaxSize * sizeof(Texcoords), Buffer::Usage::Write, Buffer::Changing::Static }, Data);

		PositionsBuffer.Load(Buffer::Properties{ MaxSize * sizeof(Vector3) * 6 }, nullptr);
		SizesBuffer.Load(Buffer::Properties{ MaxSize * sizeof(Vector3) * 6 }, nullptr);
		ColorsBuffer.Load(Buffer::Properties{ MaxSize * sizeof(Vector4) * 6 }, nullptr);
		OtherDataBuffer.Load(Buffer::Properties{ MaxSize * sizeof(Vector2) * 6 }, nullptr);
	}

	ParticlesRenderer::ParticlesRenderer(size_t MaxSize)
	{
		VerticesBuffer.CreateArray(Buffer::Properties::Default());
		TexcoordsBuffer.CreateArray(Buffer::Properties::Default());
		PositionsBuffer.CreateArray(Buffer::Properties::Default());
		SizesBuffer.CreateArray(Buffer::Properties::Default());
		ColorsBuffer.CreateArray(Buffer::Properties::Default());
		OtherDataBuffer.CreateArray(Buffer::Properties::Default());

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

		glm::quat Q(glm::vec3(Math::Radians(-MainCamera.Rot.X), Math::Radians(MainCamera.Rot.Y), 0));
		glm::mat4 Billboard = glm::mat4_cast(Q);

		Shader->SetUniform(Shader->GetFastUniform("ViewProjection"), false, MainCamera.GetViewProjection());
		Shader->SetUniformMatrix("View", &MainCamera.GetViewMatrix().M[0][0]);
		Shader->SetUniformMatrix("Projection", &MainCamera.GetProjectionMatrix().M[0][0]);
		Shader->SetUniformMatrix("Billboard", glm::value_ptr(Billboard));
		Shader->SetUniform(Shader->GetFastUniform("Frame"), iVector2(Particles.ModuleSubUV.Horizontal, Particles.ModuleSubUV.Vertical));
		Shader->SetUniform(Shader->GetFastUniform("Texture"), static_cast<TextureOpenGL*>(Mat.AlbedoMap), 0);

		Vector3* Positions = (Vector3*)PositionsBuffer.Map();
		for (size_t i = 0; i < Particles.Particles.Count; i++) for (int a = 0; a < 6; a++) Positions[i * 6 + a] = Particles.Particles.Positions[i].XYZ();
		PositionsBuffer.Unmap();

		Vector3* Sizes = (Vector3*)SizesBuffer.Map();
		for (size_t i = 0; i < Particles.Particles.Count; i++) for (int a = 0; a < 6; a++) Sizes[i * 6 + a] = Particles.Particles.Sizes[i];
		SizesBuffer.Unmap();

		Vector4* Colors = (Vector4*)ColorsBuffer.Map();
		for (size_t i = 0; i < Particles.Particles.Count; i++) for (int a = 0; a < 6; a++) Colors[i * 6 + a] = Particles.Particles.Colors[i];
		ColorsBuffer.Unmap();

		Vector2* OtherData = (Vector2*)OtherDataBuffer.Map();
		for (size_t i = 0; i < Particles.Particles.Count; i++) for (int a = 0; a < 6; a++) OtherData[i * 6 + a] = Vector2(Particles.Particles.Rotations[i], (float)Particles.Particles.Frames[i]);
		OtherDataBuffer.Unmap();

		VerticesBuffer.Bind();
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
		glEnableVertexAttribArray(0);

		TexcoordsBuffer.Bind();
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), nullptr);
		glEnableVertexAttribArray(1);

		PositionsBuffer.Bind();
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
		glEnableVertexAttribArray(2);

		SizesBuffer.Bind();
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
		glEnableVertexAttribArray(3);

		ColorsBuffer.Bind();
		glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), nullptr);
		glEnableVertexAttribArray(4);

		OtherDataBuffer.Bind();
		glVertexAttribPointer(5, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), nullptr);
		glEnableVertexAttribArray(5);

		glDrawArrays(GL_TRIANGLES, 0, Particles.Particles.Count * 6);

		glBlendEquation(GL_ADD);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}

	ParticlesRenderer::~ParticlesRenderer()
	{
		delete[] Data;
	}

}


