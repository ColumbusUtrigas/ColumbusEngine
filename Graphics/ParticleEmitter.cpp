/************************************************
*              ParticleEmitter.cpp              *
*************************************************
*          This file is a part of:              *
*               COLUMBUS ENGINE                 *
*************************************************
*                Nika(Columbus) Red             *
*                   20.07.2017                  *
*************************************************/

#include <Graphics/ParticleEmitter.h>

namespace Columbus
{

	//////////////////////////////////////////////////////////////////////////////
	C_ParticleEmitter::C_ParticleEmitter(const C_ParticleEffect* aParticleEffect) :
		mParticleEffect(const_cast<C_ParticleEffect*>(aParticleEffect)),
		mLife(0.0),
		mMaxTTL(0.0)
	{
		if (aParticleEffect == nullptr) return;

		setParticleEffect(aParticleEffect);
	}
	//////////////////////////////////////////////////////////////////////////////
	void C_ParticleEmitter::setParticleEffect(const C_ParticleEffect* aParticleEffect)
	{
		delete mBuf;
		delete mTBuf;
		delete mPBuf;
		delete mLBuf;

		mParticleEffect = const_cast<C_ParticleEffect*>(aParticleEffect);

		size_t i, j;
		float radius = mParticleEffect->getParticleShapeRadius();
		bool emitFromShell = mParticleEffect->getEmitFromShell();
		C_Vector3 box = mParticleEffect->getBoxShapeSize();

		mNoise.setOctaves(mParticleEffect->getNoiseOctaves());
		mNoise.setLacunarity(mParticleEffect->getNoiseLacunarity());
		mNoise.setPersistence(mParticleEffect->getNoisePersistence());
		mNoise.setFrequency(mParticleEffect->getNoiseFrequency());
		mNoise.setAmplitude(mParticleEffect->getNoiseAmplitude());

		for (i = 0; i < static_cast<size_t>(mParticleEffect->getParticlesCount()); i++)
		{
			C_Particle p;
			p.TTL = C_Random::range(mParticleEffect->getMinTimeToLive(), mParticleEffect->getMaxTimeToLive());
			p.velocity = C_Vector3::random(mParticleEffect->getMinVelocity(), mParticleEffect->getMaxVelocity());
			p.startPos = mParticleEffect->getPos();
			p.accel = C_Vector3::random(mParticleEffect->getMinAcceleration(), mParticleEffect->getMaxAcceleration());
			p.rotation = C_Random::range(mParticleEffect->getMinRotation(), mParticleEffect->getMaxRotation());
			p.rotationSpeed = C_Random::range(mParticleEffect->getMinRotationSpeed(), mParticleEffect->getMaxRotationSpeed());
			p.startEmitterPos = mParticleEffect->getPos();
			p.frame = static_cast<unsigned int>(C_Random::range(0, mParticleEffect->getSubUV().x * mParticleEffect->getSubUV().y));

			for (j = 0; j < 9; j++)
				p.noise[j] = C_Random::range(0, 256);

			if (p.TTL > mMaxTTL)
				mMaxTTL = p.TTL;

			switch(mParticleEffect->getParticleShape())
			{
				case C_PARTICLE_SHAPE_CIRCLE: p.genCircle(radius, emitFromShell); break;
				case C_PARTICLE_SHAPE_SPHERE: p.genSphere(radius, emitFromShell); break;
				case C_PARTICLE_SHAPE_CUBE: p.genCube(box, emitFromShell); break;
			}

			mParticles.push_back(p);
		}

		if (!mParticleEffect->getMaterial()->getShader()->isCompiled())
		{
			mParticleEffect->getMaterial()->getShader()->addAttribute("aPos", 0);
			mParticleEffect->getMaterial()->getShader()->addAttribute("aUV", 1);
			mParticleEffect->getMaterial()->getShader()->addAttribute("aPoses", 2);
			mParticleEffect->getMaterial()->getShader()->addAttribute("aTimes", 3);
			mParticleEffect->getMaterial()->getShader()->addAttribute("aColors", 4);
			mParticleEffect->getMaterial()->getShader()->compile();
		}

		mBuf = new C_Buffer(vrts, sizeof(vrts) * sizeof(float), 3);
		mTBuf = new C_Buffer(uvs, sizeof(uvs) * sizeof(float), 2);
		mCBuf = new C_Buffer();
		mPBuf = new C_Buffer();
		mLBuf = new C_Buffer();
	}
	//////////////////////////////////////////////////////////////////////////////
	C_ParticleEffect* C_ParticleEmitter::getParticleEffect() const
	{
		return mParticleEffect;
	}
	//////////////////////////////////////////////////////////////////////////////
	void C_ParticleEmitter::setCameraPos(C_Vector3 aC)
	{
		mCameraPos = aC;
	}
	//////////////////////////////////////////////////////////////////////////////
	void C_ParticleEmitter::sort()
	{
		auto func = [](const C_Particle &a, const C_Particle &b) -> bool
		{
			return a.cameraDistance > b.cameraDistance;
		};

		std::sort(mActiveParticles.begin(), mActiveParticles.end(), func);
	}
	//////////////////////////////////////////////////////////////////////////////
	void C_ParticleEmitter::setLights(std::vector<C_Light*> aLights)
	{
		mLights = aLights;
	}
	//////////////////////////////////////////////////////////////////////////////
	void C_ParticleEmitter::update(const float aTimeTick)
	{
		using namespace std;
		
		//copyActive();

		float transformation = static_cast<float>(mParticleEffect->getTransformation());
		C_Vector3 constForce = mParticleEffect->getConstantForce();
		C_Vector3 startEmitterPos = mParticleEffect->getPos();
		C_Vector4 color = mParticleEffect->getMaterial()->getColor();
		bool gradienting = static_cast<bool>(mParticleEffect->getGradienting());

		float rate = mParticleEffect->getEmitRate();
		float count = static_cast<float>(mParticleEffect->getParticlesCount());
		float fireT = 1.0f / rate;
		float spawnT = static_cast<float>(count * fireT);

		if (count <= rate * mMaxTTL)
			spawnT = mMaxTTL;

		float a = mLife;
		float percent;
		C_Vector4 up, down;
		C_Vector3 noise;
		float noiseStrength = mParticleEffect->getNoiseStrength();

		size_t counter = 0;
		/*size_t size;

		auto func = [](const C_ColorKey &a, const C_ColorKey &b) -> bool
		{
			return a.key > b.key;
		};

		std::sort(mColorKeys.begin(), mColorKeys.end(), func);*/

		down = mParticleEffect->getStartColor();
		up = mParticleEffect->getFinalColor();

		mTimer += aTimeTick;

		for (auto& Particle : mParticles)
		{
			Particle.age = 0.0;
			Particle.startEmitterPos = startEmitterPos;

			if (mTimer < fireT) continue;
			mTimer -= fireT;

			Particle.age = mTimer;
			Particle.TTL = C_Random::range(mParticleEffect->getMinTimeToLive(), mParticleEffect->getMaxTimeToLive());
			Particle.velocity = C_Vector3::random(mParticleEffect->getMinVelocity(), mParticleEffect->getMaxVelocity());
			Particle.rotationSpeed = C_Random::range(mParticleEffect->getMinRotationSpeed(), mParticleEffect->getMaxRotationSpeed());
			Particle.frame = static_cast<unsigned int>(C_Random::range(0, mParticleEffect->getSubUV().x * mParticleEffect->getSubUV().y));

			mActiveParticles.push_back(Particle);
			mParticles.erase(mParticles.begin() + counter);

			counter++;
		}

		counter = 0;

		for (auto& Particle : mActiveParticles)
		{
			if (Particle.age > Particle.TTL)
			{
				Particle.age = 0.0;

				mParticles.push_back(Particle);
				if (counter < mActiveParticles.size())
					mActiveParticles.erase(mActiveParticles.begin() + counter);
			}

			if (transformation == C_PARTICLE_TRANSFORMATION_LOCAL)
				Particle.startEmitterPos = startEmitterPos;

			percent = Particle.age / Particle.TTL;

			if (gradienting) Particle.color = down * (1 - percent) + up * percent;
			else Particle.color = color;

			noise.x = static_cast<float>(mNoise.noise(Particle.noise[0], Particle.noise[1], Particle.noise[2]));
			noise.y = static_cast<float>(mNoise.noise(Particle.noise[3], Particle.noise[4], Particle.noise[5]));
			noise.z = static_cast<float>(mNoise.noise(Particle.noise[6], Particle.noise[7], Particle.noise[8]));

			Particle.update(aTimeTick, mCameraPos, constForce, noise * noiseStrength);
			counter++;
		}

		/*for (auto& Particle : mParticles)
		{
			e = min(Particle.TTL, fireT) * counter;
			Particle.age = fmod(e + a, spawnT);

			//if (transformation == C_PARTICLE_TRANSFORMATION_LOCAL)
				Particle.startEmitterPos = startEmitterPos;
			//else
				//if (Particle.age <= aTimeTick)
					//Particle.startEmitterPos = startEmitterPos;

			prevActive = Particle.active;
			Particle.active = (Particle.age <= Particle.TTL);

			if (Particle.active == true && prevActive == false)
				if ((Particle.age / Particle.TTL) <= aTimeTick)
					Particle.startEmitterPos = startEmitterPos;

			if (Particle.active == true && prevActive == false)
			{
				Particle.TTL = C_Random::range(mParticleEffect->getMinTimeToLive(), mParticleEffect->getMaxTimeToLive());
				Particle.velocity = C_Vector3::random(mParticleEffect->getMinVelocity(), mParticleEffect->getMaxVelocity());
				Particle.rotationSpeed = C_Random::range(mParticleEffect->getMinRotationSpeed(), mParticleEffect->getMaxRotationSpeed());
				Particle.frame = C_Random::range(0, mParticleEffect->getSubUV().x * mParticleEffect->getSubUV().y);
			}

			if (Particle.active == true && Particle.age > 0)
			{
				life = fmod(Particle.age, Particle.TTL);
				percent = life / Particle.TTL;

				noise.x = static_cast<float>(mNoise.noise(Particle.noise[0], Particle.noise[1], Particle.noise[2]));
				noise.y = static_cast<float>(mNoise.noise(Particle.noise[3], Particle.noise[4], Particle.noise[5]));
				noise.z = static_cast<float>(mNoise.noise(Particle.noise[6], Particle.noise[7], Particle.noise[8]));

				if (gradienting)
					Particle.color = down * (1 - percent) + up * percent;
				else
					Particle.color = color;

				Particle.update(aTimeTick, mCameraPos, constForce, noise * noiseStrength);
			}

			counter++;
		}*/

		if (mParticleEffect->getSortMode() == C_PARTICLE_SORT_MODE_DISTANCE) sort();

		mLife += aTimeTick;
	}
	//////////////////////////////////////////////////////////////////////////////
	void C_ParticleEmitter::setBuffers()
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
	}
	//////////////////////////////////////////////////////////////////////////////
	void C_ParticleEmitter::setUniforms()
	{
		mParticleEffect->getMaterial()->getShader()->setUniform2f("uSize", mParticleEffect->getParticleSize());
		mParticleEffect->getMaterial()->getShader()->setUniform2f("uStartSize", mParticleEffect->getStartSize());
		mParticleEffect->getMaterial()->getShader()->setUniform2f("uFinalSize", mParticleEffect->getFinalSize());
		mParticleEffect->getMaterial()->getShader()->setUniform2f("uSubUV", mParticleEffect->getSubUV());
		mParticleEffect->getMaterial()->getShader()->setUniform1f("uScaleOL", static_cast<float>(mParticleEffect->getScaleOverLifetime()));
		mParticleEffect->getMaterial()->getShader()->setUniform1f("uBillboard", static_cast<float>(mParticleEffect->getBillbiarding()));
		mParticleEffect->getMaterial()->getShader()->setUniform1f("uSubUVMode", static_cast<float>(mParticleEffect->getSubUVMode()));
		mParticleEffect->getMaterial()->getShader()->setUniform1f("uSubUVCycles", static_cast<float>(mParticleEffect->getSubUVCycles()));

		mParticleEffect->getMaterial()->getShader()->setUniformMatrix("uView", C_GetViewMatrix().elements());
		mParticleEffect->getMaterial()->getShader()->setUniformMatrix("uProjection", C_GetProjectionMatrix().elements());

		if (mParticleEffect->getMaterial() != nullptr)
		{
			C_ActiveTextureOpenGL(C_OGL_TEXTURE0);
			C_BindTextureOpenGL(C_OGL_TEXTURE0, 0);

			if (mParticleEffect->getMaterial()->getTexture() != nullptr)
			{
				mParticleEffect->getMaterial()->getShader()->setUniform1i("uTex", 0);
				mParticleEffect->getMaterial()->getTexture()->sampler2D(0);
			}

			mParticleEffect->getMaterial()->getShader()->setUniform1i("uDiscard", mParticleEffect->getMaterial()->getDiscard());
		}
	}
	//////////////////////////////////////////////////////////////////////////////
	void C_ParticleEmitter::setShaderMaterial()
	{
		if (mParticleEffect == nullptr) return;
		if (mParticleEffect->getMaterial() == nullptr) return;

		C_Vector4 matcol = mParticleEffect->getMaterial()->getColor();
		C_Vector3 matamb = mParticleEffect->getMaterial()->getAmbient();
		C_Vector3 matdif = mParticleEffect->getMaterial()->getDiffuse();
		C_Vector3 matspc = mParticleEffect->getMaterial()->getSpecular();

		float const MaterialUnif[15] =
		{
			matcol.x, matcol.y, matcol.z, matcol.w,
			matamb.x, matamb.y, matamb.z,
			matdif.x, matdif.y, matdif.z,
			matspc.x, matspc.y, matspc.z,
			mParticleEffect->getMaterial()->getReflectionPower(),
			mParticleEffect->getMaterial()->getLighting() ? 1.0f : 0.0f
		};

		mParticleEffect->getMaterial()->getShader()->setUniformArrayf("MaterialUnif", MaterialUnif, 15);
	}
	//////////////////////////////////////////////////////////////////////////////
	void C_ParticleEmitter::setShaderLightAndCamera()
	{
		calculateLights();
		mParticleEffect->getMaterial()->getShader()->setUniformArrayf("LightUnif", mLightUniform, 120);
	}
	//////////////////////////////////////////////////////////////////////////////
	void C_ParticleEmitter::calculateLights()
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
				mLightUniform[0 + offset] = mLights[i]->getColor().x;
				mLightUniform[1 + offset] = mLights[i]->getColor().y;
				mLightUniform[2 + offset] = mLights[i]->getColor().z;
				//Position
				mLightUniform[3 + offset] = mLights[i]->getPos().x;
				mLightUniform[4 + offset] = mLights[i]->getPos().y;
				mLightUniform[5 + offset] = mLights[i]->getPos().z;
				//Direction
				mLightUniform[6 + offset] = mLights[i]->getDir().x;
				mLightUniform[7 + offset] = mLights[i]->getDir().y;
				mLightUniform[8 + offset] = mLights[i]->getDir().z;
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
	//////////////////////////////////////////////////////////////////////////////
	void C_ParticleEmitter::sortLights()
	{
		if (mParticleEffect == nullptr) return;

		C_Vector3 pos = mParticleEffect->getPos();

		mLights.erase(std::remove(mLights.begin(), mLights.end(), nullptr), mLights.end());

		auto func = [pos](const C_Light* a, const C_Light* b) mutable -> bool
		{
			C_Vector3 q = a->getPos();
			C_Vector3 w = b->getPos();

			return q.length(pos) < w.length(pos);
		};

		std::sort(mLights.begin(), mLights.end(), func);
	}
	//////////////////////////////////////////////////////////////////////////////
	void C_ParticleEmitter::unbindAll()
	{
		if (mParticleEffect->getAdditive())
			C_BlendFuncOpenGL(C_OGL_SRC_ALPHA, C_OGL_ONE_MINUS_SRC_ALPHA);

		C_EnableDepthMaskOpenGL();

		//C_Shader::unbind();
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
	//////////////////////////////////////////////////////////////////////////////
	void C_ParticleEmitter::draw()
	{
		if (mParticleEffect == nullptr) return;
		if (mParticleEffect->getMaterial()->getShader() == nullptr) return;
		if (mBuf == nullptr) return;
		if (mTBuf == nullptr) return;
		if (mParticleEffect->getVisible() == false) return;

		mParticleEffect->getMaterial()->getShader()->bind();

		setUniforms();

		C_DisableDepthMaskOpenGL();

		if (mParticleEffect->getAdditive())
			C_BlendFuncOpenGL(C_OGL_SRC_ALPHA, C_OGL_ONE);

		setShaderMaterial();
		setShaderLightAndCamera();

		if (mParticleEffect->getParticlesCount() != mParticlesCount)
		{
			delete mVertData;
			delete mUvData;
			delete mColData;
			delete mPosData;
			delete mTimeData;

			mParticlesCount = mParticleEffect->getParticlesCount();

			mVertData = new float[mParticlesCount * 18];
			mUvData = new float[mParticlesCount * 12];
			mColData = new float[mParticlesCount * 24];
			mPosData = new float[mParticlesCount * 18];
			mTimeData = new float[mParticlesCount * 24];

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

		for (auto Particle : mActiveParticles)
		{
			for (int i = 0; i < 6; i++)
			{
				mPosData[posCounter++] = Particle.pos.x;
				mPosData[posCounter++] = Particle.pos.y;
				mPosData[posCounter++] = Particle.pos.z;

				mTimeData[timeCounter++] = Particle.age;
				mTimeData[timeCounter++] = Particle.TTL;
				mTimeData[timeCounter++] = Particle.rotation;
				mTimeData[timeCounter++] = static_cast<float>(Particle.frame);

				mColData[colCounter++] = Particle.color.x;
				mColData[colCounter++] = Particle.color.y;
				mColData[colCounter++] = Particle.color.z;
				mColData[colCounter++] = Particle.color.w;
			}
		}

		mBuf->setData(mVertData, 18 * sizeof(float) * mActiveParticles.size(), 3);
		mBuf->compile();

		mTBuf->setData(mUvData, 12 * sizeof(float) * mActiveParticles.size(), 2);
		mTBuf->compile();

		mCBuf->setData(mColData, 24 * sizeof(float) * mActiveParticles.size(), 4);
		mCBuf->compile();

		mPBuf->setData(mPosData, 18 * sizeof(float)* mActiveParticles.size(), 3);
		mPBuf->compile();

		mLBuf->setData(mTimeData, 24 * sizeof(float) * mActiveParticles.size(), 4);
		mLBuf->compile();

		setBuffers();

		C_DrawArraysOpenGL(C_OGL_TRIANGLES, 0, 6 * mActiveParticles.size());

		unbindAll();
	}
	//////////////////////////////////////////////////////////////////////////////
	C_ParticleEmitter::~C_ParticleEmitter()
	{
		mParticles.clear();
		mActiveParticles.clear();
		
		if (mVertData != nullptr) delete mVertData;
		if (mUvData != nullptr) delete mUvData;
		if (mUvData != nullptr) delete mPosData;
		if (mUvData != nullptr) delete mTimeData;
	}

}
