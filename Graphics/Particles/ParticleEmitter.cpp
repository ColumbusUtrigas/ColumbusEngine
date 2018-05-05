/************************************************
*              ParticleEmitter.cpp              *
*************************************************
*          This file is a part of:              *
*               COLUMBUS ENGINE                 *
*************************************************
*                Nika(Columbus) Red             *
*                   20.07.2017                  *
*************************************************/

#include <Graphics/Particles/ParticleEmitter.h>

namespace Columbus
{

	ParticleEmitter::ParticleEmitter(const ParticleEffect* aParticleEffect) :
		mParticleEffect(const_cast<ParticleEffect*>(aParticleEffect)),
		mLife(0.0)
	{
		if (aParticleEffect == nullptr) return;

		setParticleEffect(aParticleEffect);
	}
	
	void ParticleEmitter::setParticleEffect(const ParticleEffect* aParticleEffect)
	{
		if (aParticleEffect == nullptr) return;

		delete mBuf;
		delete mTBuf;
		delete mPBuf;
		delete mLBuf;
		delete mSBuf;

		mParticleEffect = const_cast<ParticleEffect*>(aParticleEffect);
		if (mParticleEffect->Emit == nullptr) return;

		mParticles.resize(mParticleEffect->Emit->Count);
		Particle p;

		for (auto& Particle : mParticles)
		{
			for (uint32 j = 0; j < 9; j++)
			{
				p.noise[j] = Random::range(0, 256);
			}

			Particle = p;
		}

		if (!mParticleEffect->getMaterial()->getShader()->IsCompiled())
		{
			auto tShader = mParticleEffect->getMaterial()->getShader();

			tShader->AddAttribute("aPos", 0);
			tShader->AddAttribute("aUV", 1);
			tShader->AddAttribute("aPoses", 2);
			tShader->AddAttribute("aTimes", 3);
			tShader->AddAttribute("aColors", 4);
			tShader->AddAttribute("aSizes", 5);
			tShader->Compile();

			tShader->AddUniform("uBillboard");
			tShader->AddUniform("uSubUV");
			tShader->AddUniform("uView");
			tShader->AddUniform("uProjection");
			tShader->AddUniform("uTex");
			tShader->AddUniform("uDiscard");
			tShader->AddUniform("MaterialUnif");
			tShader->AddUniform("LightUnif");
		}

		mBuf = new C_Buffer(vrts, sizeof(vrts) * sizeof(float), 3);
		mTBuf = new C_Buffer(uvs, sizeof(uvs) * sizeof(float), 2);
		mCBuf = new C_Buffer();
		mPBuf = new C_Buffer();
		mLBuf = new C_Buffer();
		mSBuf = new C_Buffer();
	}
	
	ParticleEffect* ParticleEmitter::getParticleEffect() const
	{
		return mParticleEffect;
	}
	
	void ParticleEmitter::setCamera(const Camera aCamera)
	{
		mCamera = static_cast<Camera>(aCamera);
	}
	
	void ParticleEmitter::sort()
	{
		auto func = [](const Particle &a, const Particle &b) -> bool
		{
			return a.cameraDistance > b.cameraDistance;
		};

		std::sort(mActiveParticles.begin(), mActiveParticles.end(), func);
	}
	
	void ParticleEmitter::setLights(std::vector<Light*> aLights)
	{
		mLights = aLights;
	}
	
	void ParticleEmitter::update(const float aTimeTick)
	{
		Vector3 startEmitterPos = mParticleEffect->getPos();

		if (mParticleEffect->Emit == nullptr ||
		    mParticleEffect->Required == nullptr) return;

		float rate = mParticleEffect->Emit->EmitRate;
		float fireT = 1.0f / rate;
		size_t counter = 0;

		mTimer += aTimeTick;

		if (mParticles.size() == 0)
		{
			mTimer = 0.0f;
		}

		if (mParticleEffect->Emit->Active)
		{
			for (auto& Particle : mParticles)
			{
				Particle.age = 0.0;
				Particle.startEmitterPos = startEmitterPos;

				if (mTimer < fireT || mParticles.size() == 0) continue;
				mTimer -= fireT;

				Particle.age = mTimer;

				for (auto& Module : mParticleEffect->Modules)
				{
					Module->Spawn(Particle);
				}

				mActiveParticles.push_back(Particle);
				mParticles.erase(mParticles.begin() + counter);

				counter++;
			}
		}

		counter = 0;

		for (auto& Particle : mActiveParticles)
		{
			if (Particle.age > Particle.TTL)
			{
				Particle.age = 0.0;

				mParticles.push_back(Particle);
				if (counter < mActiveParticles.size())
				{
					mActiveParticles.erase(mActiveParticles.begin() + counter);
				}
			}

			if (mParticleEffect->Required->Transformation == E_PARTICLE_TRANSFORMATION_LOCAL)
			{
				Particle.startEmitterPos = startEmitterPos;
			}

			for (auto& Module : mParticleEffect->Modules)
			{
				Module->Update(Particle);
			}

			Particle.update(aTimeTick, mCamera.getPos());
			
			counter++;
		}

		if (mParticleEffect->Required->SortMode == E_PARTICLE_SORT_MODE_DISTANCE)
		{
			sort();
		}

		mLife += aTimeTick;
	}
	
	void ParticleEmitter::setBuffers()
	{
		mBuf->bind();
		C_VertexAttribPointerOpenGL(0, 3, C_OGL_FLOAT, C_OGL_FALSE, 3 * sizeof(float), NULL);
		C_OpenStreamOpenGL(0);
		mTBuf->bind();
		C_VertexAttribPointerOpenGL(1, 2, C_OGL_FLOAT, C_OGL_FALSE, 2 * sizeof(float), NULL);
		C_OpenStreamOpenGL(1);
		mPBuf->bind();
		C_VertexAttribPointerOpenGL(2, 3, C_OGL_FLOAT, C_OGL_FALSE, 3 * sizeof(float), NULL);
		C_OpenStreamOpenGL(2);
		mLBuf->bind();
		C_VertexAttribPointerOpenGL(3, 4, C_OGL_FLOAT, C_OGL_FALSE, 4 * sizeof(float), NULL);
		C_OpenStreamOpenGL(3);
		mCBuf->bind();
		C_VertexAttribPointerOpenGL(4, 4, C_OGL_FLOAT, C_OGL_FALSE, 4 * sizeof(float), NULL);
		C_OpenStreamOpenGL(4);
		mSBuf->bind();
		C_VertexAttribPointerOpenGL(5, 3, C_OGL_FLOAT, C_OGL_FALSE, 3 * sizeof(float), NULL);
		C_OpenStreamOpenGL(5);
	}
	
	void ParticleEmitter::setUniforms()
	{
		ParticleModuleSubUV* SubUV = static_cast<ParticleModuleSubUV*>(mParticleEffect->GetModule(E_PARTICLE_MODULE_SUBUV));

		if (mParticleEffect->Required == nullptr ||
		    SubUV == nullptr) return;

		mParticleEffect->getMaterial()->getShader()->SetUniform1f("uBillboard", static_cast<float>(mParticleEffect->Required->Billboarding));
		mParticleEffect->getMaterial()->getShader()->SetUniform2f("uSubUV", Vector2(SubUV->Horizontal, SubUV->Vertical));

		mCamera.getViewMatrix().Elements(UniformViewMatrix);
		mCamera.getProjectionMatrix().ElementsTransposed(UniformProjectionMatrix);

		mParticleEffect->getMaterial()->getShader()->SetUniformMatrix("uView", UniformViewMatrix);
		mParticleEffect->getMaterial()->getShader()->SetUniformMatrix("uProjection", UniformProjectionMatrix);

		if (mParticleEffect->getMaterial() != nullptr)
		{
			C_ActiveTextureOpenGL(C_OGL_TEXTURE0);
			C_BindTextureOpenGL(C_OGL_TEXTURE0, 0);

			if (mParticleEffect->getMaterial()->getTexture() != nullptr)
			{
				mParticleEffect->getMaterial()->getShader()->SetUniform1i("uTex", 0);
				mParticleEffect->getMaterial()->getTexture()->sampler2D(0);
			}

			mParticleEffect->getMaterial()->getShader()->SetUniform1i("uDiscard", mParticleEffect->getMaterial()->getDiscard());
		}
	}
	
	void ParticleEmitter::setShaderMaterial()
	{
		if (mParticleEffect == nullptr) return;
		if (mParticleEffect->getMaterial() == nullptr) return;

		Vector4 matcol = mParticleEffect->getMaterial()->getColor();
		Vector3 matamb = mParticleEffect->getMaterial()->getAmbient();
		Vector3 matdif = mParticleEffect->getMaterial()->getDiffuse();
		Vector3 matspc = mParticleEffect->getMaterial()->getSpecular();

		float const MaterialUnif[15] =
		{
			matcol.x, matcol.y, matcol.z, matcol.w,
			matamb.X, matamb.Y, matamb.Z,
			matdif.X, matdif.Y, matdif.Z,
			matspc.X, matspc.Y, matspc.Z,
			mParticleEffect->getMaterial()->getReflectionPower(),
			mParticleEffect->getMaterial()->getLighting() ? 1.0f : 0.0f
		};

		mParticleEffect->getMaterial()->getShader()->SetUniformArrayf("MaterialUnif", MaterialUnif, 15);
	}
	
	void ParticleEmitter::setShaderLightAndCamera()
	{
		calculateLights();
		mParticleEffect->getMaterial()->getShader()->SetUniformArrayf("LightUnif", mLightUniform, 120);
	}
	
	void ParticleEmitter::calculateLights()
	{
		sortLights();
		size_t i, j, offset;
		//8 - max count of lights, processing in shader
		for (i = 0; i < 8; i++)
		{
			offset = i * 15;

			if (i < mLights.size() && mParticleEffect->getMaterial()->getLighting() == true)
			{
				//Color
				mLightUniform[0 + offset] = mLights[i]->getColor().X;
				mLightUniform[1 + offset] = mLights[i]->getColor().Y;
				mLightUniform[2 + offset] = mLights[i]->getColor().Z;
				//Position
				mLightUniform[3 + offset] = mLights[i]->getPos().X;
				mLightUniform[4 + offset] = mLights[i]->getPos().Y;
				mLightUniform[5 + offset] = mLights[i]->getPos().Z;
				//Direction
				mLightUniform[6 + offset] = mLights[i]->getDir().X;
				mLightUniform[7 + offset] = mLights[i]->getDir().Y;
				mLightUniform[8 + offset] = mLights[i]->getDir().Z;
				//Type
				mLightUniform[9 + offset] = static_cast<float>(mLights[i]->getType());
				//Constant attenuation
				mLightUniform[10 + offset] = mLights[i]->getConstant();
				//Linear attenuation
				mLightUniform[11 + offset] = mLights[i]->getLinear();
				//Quadratic attenuation
				mLightUniform[12 + offset] = mLights[i]->getQuadratic();
				//Inner cutoff
				mLightUniform[13 + offset] = mLights[i]->getInnerCutoff();
				//Outer cutoff
				mLightUniform[14 + offset] = mLights[i]->getOuterCutoff();
			} else
			{
				for (j = 0; j < 15; j++)
					mLightUniform[j + offset] = -1;
			}
		}
	}
	
	void ParticleEmitter::sortLights()
	{
		if (mParticleEffect == nullptr) return;

		Vector3 pos = mParticleEffect->getPos();

		mLights.erase(std::remove(mLights.begin(), mLights.end(), nullptr), mLights.end());

		auto func = [pos](const Light* a, const Light* b) mutable -> bool
		{
			Vector3 q = a->getPos();
			Vector3 w = b->getPos();

			return q.Length(pos) < w.Length(pos);
		};

		std::sort(mLights.begin(), mLights.end(), func);
	}
	
	void ParticleEmitter::unbindAll()
	{
		if (mParticleEffect->Required == nullptr) return;

		if (mParticleEffect->Required->AdditiveBlending)
		{
			C_BlendFuncOpenGL(C_OGL_SRC_ALPHA, C_OGL_ONE_MINUS_SRC_ALPHA);
		}

		C_EnableDepthMaskOpenGL();

		//Shader::unbind();
		//C_Texture::unbind();
		glUseProgram(0);
		glBindTexture(GL_TEXTURE_2D, 0);
		C_Buffer::unbind();

		C_CloseStreamOpenGL(0);
		C_CloseStreamOpenGL(1);
		C_CloseStreamOpenGL(2);
		C_CloseStreamOpenGL(3);
		C_CloseStreamOpenGL(4);
	}
	
	void ParticleEmitter::draw()
	{
		if (mParticleEffect == nullptr) return;
		if (mParticleEffect->getMaterial()->getShader() == nullptr) return;
		if (mBuf == nullptr) return;
		if (mTBuf == nullptr) return;

		if (mParticleEffect->Emit == nullptr ||
			mParticleEffect->Required == nullptr) return;

		if (mParticleEffect->Required->Visible == false) return;

		mParticleEffect->getMaterial()->getShader()->Bind();

		setUniforms();

		C_DisableDepthMaskOpenGL();

		if (mParticleEffect->Required->AdditiveBlending)
		{
			C_BlendFuncOpenGL(C_OGL_SRC_ALPHA, C_OGL_ONE);
		}

		setShaderMaterial();
		setShaderLightAndCamera();

		if (mParticleEffect->Emit->Count != mParticlesCount)
		{
			delete[] mVertData;
			delete[] mUvData;
			delete[] mColData;
			delete[] mPosData;
			delete[] mTimeData;
			delete[] mSizeData;

			mParticlesCount = mParticleEffect->Emit->Count;

			mVertData = new float[mParticlesCount * 18];
			mUvData = new float[mParticlesCount * 12];
			mColData = new float[mParticlesCount * 24];
			mPosData = new float[mParticlesCount * 18];
			mTimeData = new float[mParticlesCount * 24];
			mSizeData = new float[mParticlesCount * 18];

			unsigned int vertCounter = 0;
			unsigned int uvCounter = 0;

			for (size_t i = 0; i < mParticlesCount; i++)
			{
				memcpy(mVertData + vertCounter, vrts, sizeof(vrts));
				vertCounter += 18;

				memcpy(mUvData + uvCounter, uvs, sizeof(uvs));
				uvCounter += 12;
			}
		}

		unsigned int posCounter = 0;
		unsigned int timeCounter = 0;
		unsigned int colCounter = 0;
		unsigned int sizeCounter = 0;
		unsigned int counter = 0;

		for (auto Particle : mActiveParticles)
		{
			if (counter >= mActiveParticles.size()) break;
			counter++;

			for (int i = 0; i < 6; i++)
			{
				mPosData[posCounter++] = Particle.pos.X;
				mPosData[posCounter++] = Particle.pos.Y;
				mPosData[posCounter++] = Particle.pos.Z;

				mTimeData[timeCounter++] = Particle.age;
				mTimeData[timeCounter++] = Particle.TTL;
				mTimeData[timeCounter++] = Particle.rotation;
				mTimeData[timeCounter++] = static_cast<float>(Particle.frame);

				mColData[colCounter++] = Particle.color.x;
				mColData[colCounter++] = Particle.color.y;
				mColData[colCounter++] = Particle.color.z;
				mColData[colCounter++] = Particle.color.w;

				mSizeData[sizeCounter++] = Particle.size.X;
				mSizeData[sizeCounter++] = Particle.size.Y;
				mSizeData[sizeCounter++] = Particle.size.Z;
			}

			mBuf->setData(mVertData, 18 * sizeof(float) * mActiveParticles.size(), 3);
			mBuf->compile();

			mTBuf->setData(mUvData, 12 * sizeof(float) * mActiveParticles.size(), 2);
			mTBuf->compile();
		}

		mCBuf->setData(mColData, 24 * sizeof(float) * mActiveParticles.size(), 4);
		mCBuf->compile();

		mPBuf->setData(mPosData, 18 * sizeof(float)* mActiveParticles.size(), 3);
		mPBuf->compile();

		mLBuf->setData(mTimeData, 24 * sizeof(float) * mActiveParticles.size(), 4);
		mLBuf->compile();

		mSBuf->setData(mSizeData, 18 * sizeof(float) * mActiveParticles.size(), 3);
		mSBuf->compile();

		setBuffers();

		C_DrawArraysOpenGL(C_OGL_TRIANGLES, 0, 6 * mActiveParticles.size());

		unbindAll();
	}
	
	ParticleEmitter::~ParticleEmitter()
	{
		mParticles.clear();
		mActiveParticles.clear();
		
		delete mVertData;
		delete mUvData;
		delete mPosData;
		delete mTimeData;
		delete mSizeData;
	}

}



