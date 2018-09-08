#include <Graphics/Particles/ParticleEmitter.h>
#include <Graphics/Device.h>

#include <Graphics/OpenGL/BufferOpenGL.h>

namespace Columbus
{

	static float vrts[18] =
	{
		1, 1, 0.0,
		-1, 1, 0.0,
		-1, -1, 0.0,
		-1, -1, 0.0,
		1, -1, 0.0,
		1, 1, 0.0
	};

	static float uvs[12] =
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
	
	void ParticleEmitter::SetCamera(const Camera& InCamera)
	{
		ObjectCamera = InCamera;
	}
	
	void ParticleEmitter::Update(float TimeTick)
	{
		if (Effect != nullptr)
		{
			Vector3 startEmitterPos = Effect->getPos();

			if (Effect->Emit != nullptr && Effect->Required != nullptr)
			{
				float rate = Effect->Emit->EmitRate;
				float fireT = 1.0f / rate;
				size_t counter = 0;

				Timer += TimeTick;

				if (Effect->Emit->Count == 0)
				{
					Timer = 0.0f;
				}

				//Spawning new particles
				if (Effect->Emit->Active)
				{
					while (Timer >= fireT && Particles.size() <= Effect->Emit->Count)
					{
						Timer -= fireT;

						Particle NewParticle;
						NewParticle.age = Timer;
						NewParticle.startEmitterPos = startEmitterPos;

						for (uint32 j = 0; j < 9; j++)
						{
							NewParticle.noise[j] = Random::Range(0.0f, 256.0f);
						}

						for (auto& Module : Effect->Modules)
						{
							Module->Spawn(NewParticle);
						}

						Particles.push_back(NewParticle);
					}
				}

				//Updating active particles
				counter = 0;

				for (auto& Particle : Particles)
				{
					if (Effect->Required->Transformation == ParticleTransformation::Local)
					{
						Particle.startEmitterPos = startEmitterPos;
					}

					for (auto& Module : Effect->Modules)
					{
						Module->Update(Particle);
					}

					Particle.update(TimeTick, ObjectCamera.getPos());
					
					counter++;
				}

				//Deleting dead particles
				counter = 0;

				for (auto& Particle : Particles)
				{
					if (Particle.age >= Particle.TTL)
					{
						if (counter < Particles.size())
						{
							Particles.erase(Particles.begin() + counter);
						}
					}

					counter++;
				}

				if (Effect->Required->SortMode == ParticleSortMode::Distance)
				{
					static auto Sorter = [](const Particle& A, const Particle& B)->bool
					{
						return A.cameraDistance > A.cameraDistance;
					};

					std::sort(Particles.begin(), Particles.end(), Sorter);
				}

				Life += TimeTick;
			}
		}
	}

	void ParticleEmitter::UpdateMainBuffers()
	{
		if (Effect != nullptr)
		{
			delete[] VertData;
			delete[] UVData;
			delete[] PositionData;
			delete[] TimeData;
			delete[] ColorData;
			delete[] SizeData;

			ParticlesCount = Effect->Emit->Count;

			VertData = new float[ParticlesCount * 18];
			UVData = new float[ParticlesCount * 12];

			PositionData = new Vector3[ParticlesCount * 6];
			TimeData = new Vector2[ParticlesCount * 6];
			ColorData = new Vector4[ParticlesCount * 6];
			SizeData = new Vector3[ParticlesCount * 6];

			uint32 VertCounter = 0;
			uint32 UVCounter = 0;

			for (uint32 i = 0; i < ParticlesCount; i++)
			{
				Memory::Memcpy(VertData + VertCounter, vrts, sizeof(vrts));
				VertCounter += 18;

				Memory::Memcpy(UVData + UVCounter, uvs, sizeof(uvs));
				UVCounter += 12;
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
		ParticleModuleSubUV* SubUV = static_cast<ParticleModuleSubUV*>(Effect->GetModule(ParticleModule::Type::SubUV));

		if (Effect->Required != nullptr && SubUV != nullptr)
		{
			static float UniformViewMatrix[16];
			static float UniformProjectionMatrix[16];

			Effect->getMaterial()->GetShader()->SetUniform1f("uBillboard", static_cast<float>(Effect->Required->Billboarding));
			Effect->getMaterial()->GetShader()->SetUniform2f("uSubUV", Vector2(SubUV->Horizontal, SubUV->Vertical));

			ObjectCamera.getViewMatrix().Elements(UniformViewMatrix);
			ObjectCamera.getProjectionMatrix().Elements(UniformProjectionMatrix);

			Effect->getMaterial()->GetShader()->SetUniformMatrix("uView", UniformViewMatrix);
			Effect->getMaterial()->GetShader()->SetUniformMatrix("uProjection", UniformProjectionMatrix);

			if (Effect->getMaterial() != nullptr)
			{
				if (Effect->getMaterial()->DiffuseTexture != nullptr)
				{
					Effect->getMaterial()->GetShader()->SetUniform1i("uTex", 0);
					Effect->getMaterial()->DiffuseTexture->sampler2D(0);
				}
				else
				{
					glActiveTexture(GL_TEXTURE0);
					glBindTexture(GL_TEXTURE_2D, 0);
				}
			}
		}
	}
	
	void ParticleEmitter::Render()
	{
		if (Effect != nullptr)
		{
			if (Effect->getMaterial()->GetShader() != nullptr)
			{
				if (Effect->Emit != nullptr && Effect->Required != nullptr)
				{
					if (Effect->Required->Visible)
					{
						if (!Effect->getMaterial()->GetShader()->IsError())
						{
							if (!Effect->getMaterial()->GetShader()->IsCompiled())
							{
								if (!Effect->getMaterial()->GetShader()->Compile()) return;
							}

							Effect->getMaterial()->GetShader()->Bind();
						} else return;

						if (Effect->Required->AdditiveBlending)
						{
							glBlendFunc(GL_SRC_ALPHA, GL_ONE);
						}

						if (Effect->Emit->Count != ParticlesCount)
						{
							UpdateMainBuffers();
						}

						uint64 PositionsCounter = 0;
						uint64 TimesCounter = 0;
						uint64 ColorsCounter = 0;
						uint64 SizesCounter = 0;
						uint64 Counter = 0;

						for (const auto& Particle : Particles)
						{
							if (Counter >= Particles.size()) break;
							Counter++;

							for (int i = 0; i < 6; i++)
							{
								PositionData[PositionsCounter++] = Particle.pos;
								TimeData[TimesCounter++] = Vector2(Particle.rotation, Particle.frame);
								ColorData[ColorsCounter++] = Particle.Color;
								SizeData[SizesCounter++] = Particle.Size;
							}
						}

						VerticesBuffer->Load(Buffer::Properties{ 18 * sizeof(float) * ParticlesCount }, VertData);
						UVBuffer->Load(Buffer::Properties{ 12 * sizeof(float) * ParticlesCount }, UVData);

						Positions->Load(Buffer::Properties{ 18 * sizeof(float) * Particles.size(), Buffer::Usage::Write, Buffer::Changing::Static }, PositionData);
						Times->Load(Buffer::Properties{ 12 * sizeof(float) * Particles.size(), Buffer::Usage::Write, Buffer::Changing::Static }, TimeData);
						Colors->Load(Buffer::Properties{ 24 * sizeof(float) * Particles.size(), Buffer::Usage::Write, Buffer::Changing::Static }, ColorData);
						Sizes->Load(Buffer::Properties{ 18 * sizeof(float) * Particles.size(), Buffer::Usage::Write, Buffer::Changing::Static }, SizeData);

						SetBuffers();
						SetUniforms();

						glDrawArrays(GL_TRIANGLES, 0, 6 * Particles.size());

						if (Effect->Required->AdditiveBlending)
						{
							glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
						}
					}
				}
			}
		}
	}
	
	ParticleEmitter::~ParticleEmitter()
	{
		Particles.clear();
		
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



