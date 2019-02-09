#include <Graphics/Particles/ParticleEmitter.h>
#include <Graphics/Particles/SubUV/ParticleModuleSubUV.h>
#include <Graphics/Device.h>

#include <Graphics/OpenGL/BufferOpenGL.h>
#include <Graphics/OpenGL/ShaderOpenGL.h>
#include <Core/Random.h>
#include <GL/glew.h>
#include <algorithm>

namespace Columbus
{

	static float Vertices[18] =
	{
		1, 1, 0.0,
		-1, 1, 0.0,
		-1, -1, 0.0,
		-1, -1, 0.0,
		1, -1, 0.0,
		1, 1, 0.0
	};

	static float Texcoords[12] =
	{
		1.0, 1.0,
		0.0, 1.0,
		0.0, 0.0,
		0.0, 0.0,
		1.0, 0.0,
		1.0, 1.0
	};

	ParticleEmitter::ParticleEmitter(ParticleEffect* InEffect) : Life(0.0f)
	{
		SetParticleEffect(InEffect);

		VerticesBuffer = gDevice->CreateBuffer();
		UVBuffer = gDevice->CreateBuffer();
		Positions = gDevice->CreateBuffer();
		Times = gDevice->CreateBuffer();
		Colors = gDevice->CreateBuffer();
		Sizes = gDevice->CreateBuffer();

		VerticesBuffer->CreateArray(Buffer::Properties::Default());
		UVBuffer->CreateArray(Buffer::Properties::Default());
		Positions->CreateArray(Buffer::Properties{0, Buffer::Usage::Write, Buffer::Changing::Stream});
		Times->CreateArray(Buffer::Properties{0, Buffer::Usage::Write, Buffer::Changing::Stream});
		Colors->CreateArray(Buffer::Properties{0, Buffer::Usage::Write, Buffer::Changing::Stream});
		Sizes->CreateArray(Buffer::Properties{0, Buffer::Usage::Write, Buffer::Changing::Stream});
	}
	
	void ParticleEmitter::SetParticleEffect(ParticleEffect* InEffect)
	{
		if (InEffect != nullptr)
		{
			Effect = InEffect;
		}
	}
	
	ParticleEffect* ParticleEmitter::GetParticleEffect() const
	{
		return Effect;
	}
	
	void ParticleEmitter::Update(float TimeTick)
	{
		if (Effect != nullptr)
		{
			Vector3 StartEmitterPos = Effect->Position;

			float Rate = Effect->Emit.EmitRate;
			float FireTime = 1.0f / Rate;

			Timer += TimeTick;

			if (Effect->Emit.Count == 0)
			{
				Timer = 0.0f;
			}

			auto ParticleSpawner = [&](Particle& P, float Timer)
			{
				P.age = Timer;
				P.startEmitterPos = StartEmitterPos;
				P.Alive = true;

				for (uint32 j = 0; j < 9; j++)
				{
					P.noise[j] = Random::Range(0.0f, 256.0f);
				}

				for (auto& Module : Effect->Modules)
				{
					Module->Spawn(P);
				}
			};

			//Spawning new particles
			if (Effect->Emit.Active)
			{
				while (Timer >= FireTime && ActiveCount <= MaxCount)
				{
					Timer -= FireTime;

					auto It = std::find_if(&Particles[0], &Particles[MaxCount], [](const Particle& A) { return !A.Alive; });
					if (It != nullptr)
					{
						ParticleSpawner(*It, Timer);
						ActiveCount++;
					}
				}
			}

			for (uint32 i = 0; i < MaxCount; i++)
			{
				if (Particles[i].Alive)
				{
					if (Effect->Required.Transformation == ParticleTransformation::Local)
					{
						Particles[i].startEmitterPos = StartEmitterPos;
					}

					for (auto& Module : Effect->Modules)
					{
						Module->Update(Particles[i]);
					}

					Particles[i].update(TimeTick, ObjectCamera.Pos);
				}
			}

			//Deleting dead particles
			if (ActiveCount != 0)
			{
				for (uint32 i = 0; i < MaxCount; i++)
				{
					if (Particles[i].age >= Particles[i].TTL && Particles[i].Alive)
					{
						Particles[i].Alive = false;
						ActiveCount--;
					}
				}
			}

			switch (Effect->Required.SortMode)
			{
			case ParticleSortMode::None: break;
			case ParticleSortMode::Distance:   std::sort(&Particles[0], &Particles[MaxCount], [](const auto& A, const auto& B) { return A.cameraDistance > B.cameraDistance; }); break;
			case ParticleSortMode::YoungFirst: std::sort(&Particles[0], &Particles[MaxCount], [](const auto& A, const auto& B) { return A.age > B.age; }); break;
			case ParticleSortMode::OldFirst:   std::sort(&Particles[0], &Particles[MaxCount], [](const auto& A, const auto& B) { return A.age < B.age; }); break;
			}

			Life += TimeTick;
		}
	}

	void ParticleEmitter::UpdateMainBuffers()
	{
		if (Effect != nullptr)
		{
			delete[] Particles;
			delete[] VertData;
			delete[] UVData;
			delete[] PositionData;
			delete[] TimeData;
			delete[] ColorData;
			delete[] SizeData;

			MaxCount = Effect->Emit.Count;

			Particles = new Particle[MaxCount];

			VertData = new Vector3[MaxCount * 6];
			UVData = new Vector2[MaxCount * 6];

			PositionData = new Vector3[MaxCount * 6];
			TimeData = new Vector2[MaxCount * 6];
			ColorData = new Vector4[MaxCount * 6];
			SizeData = new Vector3[MaxCount * 6];

			for (uint32 i = 0; i < MaxCount * 6; i += 6)
			{
				memcpy(VertData + i, Vertices, sizeof(Vertices));
				memcpy(UVData + i, Texcoords, sizeof(Texcoords));
			}
		}
	}
	
	void ParticleEmitter::SetBuffers()
	{
		static_cast<BufferOpenGL*>(VerticesBuffer)->Bind();
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
		glEnableVertexAttribArray(0);

		static_cast<BufferOpenGL*>(UVBuffer)->Bind();
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), nullptr);
		glEnableVertexAttribArray(1);

		static_cast<BufferOpenGL*>(Positions)->Bind();
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
		glEnableVertexAttribArray(2);

		static_cast<BufferOpenGL*>(Times)->Bind();
		glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), nullptr);
		glEnableVertexAttribArray(3);

		static_cast<BufferOpenGL*>(Colors)->Bind();
		glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), nullptr);
		glEnableVertexAttribArray(4);

		static_cast<BufferOpenGL*>(Sizes)->Bind();
		glVertexAttribPointer(5, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
		glEnableVertexAttribArray(5);
	}
	
	void ParticleEmitter::SetUniforms()
	{
		ParticleModuleSubUV* SubUV = (ParticleModuleSubUV*)Effect->GetModule(ParticleModule::Type::SubUV);
		ShaderProgramOpenGL* Shader = (ShaderProgramOpenGL*)Effect->Material.GetShader();

		Shader->SetUniform(Shader->GetFastUniform("uBillboard"), (float)Effect->Required.Billboarding);

		if (SubUV != nullptr)
		{
			Shader->SetUniform(Shader->GetFastUniform("uSubUV"), iVector2(SubUV->Horizontal, SubUV->Vertical));
		}
		else
		{
			Shader->SetUniform(Shader->GetFastUniform("uSubUV"), iVector2(1, 1));
		}

		Shader->SetUniformMatrix("uView", &ObjectCamera.GetViewMatrix().M[0][0]);
		Shader->SetUniformMatrix("uProjection", &ObjectCamera.GetProjectionMatrix().M[0][0]);

		if (Effect->Material.AlbedoMap != nullptr)
		{
			Shader->SetUniform(Shader->GetFastUniform("uTex"), (TextureOpenGL*)Effect->Material.AlbedoMap, 0);
		}
		else
		{
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, 0);
		}
	}
	
	void ParticleEmitter::Render()
	{
		Material Mat = Effect->Material;
		ShaderProgram* Shader = Mat.GetShader();

		if (Shader != nullptr)
		{
			if (Shader->IsError())
			{
				return;
			}

			if (Effect->Required.Visible)
			{
				if (Effect->Required.AdditiveBlending)
				{
					glBlendFunc(GL_SRC_ALPHA, GL_ONE);
				}

				if (Effect->Emit.Count != MaxCount)
				{
					UpdateMainBuffers();
				}

				uint64 PositionsCounter = 0;
				uint64 TimesCounter = 0;
				uint64 ColorsCounter = 0;
				uint64 SizesCounter = 0;
				uint64 Counter = 0;

				for (uint32 i = 0; i < MaxCount; i++)
				{
					if (Particles[i].Alive)
					{
						for (int j = 0; j < 6; j++)
						{
							PositionData[PositionsCounter++] = Particles[i].pos;
							TimeData[TimesCounter++] = Vector2(Particles[i].rotation, (float)Particles[i].frame);
							ColorData[ColorsCounter++] = Particles[i].Color;
							SizeData[SizesCounter++] = Particles[i].Size;
						}
					}
				}

				VerticesBuffer->Load(Buffer::Properties{ sizeof(Vector3) * MaxCount * 6 }, VertData);
				UVBuffer->Load(Buffer::Properties{ sizeof(Vector2) * MaxCount * 6 }, UVData);

				Positions->Load(Buffer::Properties{ sizeof(Vector3) * ActiveCount * 6, Buffer::Usage::Write, Buffer::Changing::Static }, PositionData);
				Times->Load(Buffer::Properties{ sizeof(Vector2) * ActiveCount * 6, Buffer::Usage::Write, Buffer::Changing::Static }, TimeData);
				Colors->Load(Buffer::Properties{ sizeof(Vector4) * ActiveCount * 6, Buffer::Usage::Write, Buffer::Changing::Static }, ColorData);
				Sizes->Load(Buffer::Properties{ sizeof(Vector3) * ActiveCount * 6, Buffer::Usage::Write, Buffer::Changing::Static }, SizeData);

				SetBuffers();
				SetUniforms();

				glDrawArrays(GL_TRIANGLES, 0, 6 * ActiveCount);

				if (Effect->Required.AdditiveBlending)
				{
					glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				}
			}
		}
	}
	
	ParticleEmitter::~ParticleEmitter()
	{
		delete[] Particles;

		delete[] VertData;
		delete[] UVData;
		delete[] PositionData;
		delete[] TimeData;
		delete[] ColorData;
		delete[] SizeData;

		delete VerticesBuffer;
		delete UVBuffer;
		delete Positions;
		delete Times;
		delete Colors;
		delete Sizes;
	}

}




