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
	//Constructor
	C_ParticleEmitter::C_ParticleEmitter(const C_ParticleEffect* aParticleEffect) :
		mParticleEffect(const_cast<C_ParticleEffect*>(aParticleEffect)),
		mLife(0.0),
		mMaxTTL(0.0)
	{
		if (aParticleEffect == nullptr)
			return;

		size_t i;
		float ang, rad,phi, tht;
		float xsp, ysp, zsp;

		mNoise.setOctaves(mParticleEffect->getNoiseOctaves());
		mNoise.setLacunarity(mParticleEffect->getNoiseLacunarity());
		mNoise.setPersistence(mParticleEffect->getNoisePersistence());
		mNoise.setFrequency(mParticleEffect->getNoiseFrequency());
		mNoise.setAmplitude(mParticleEffect->getNoiseAmplitude());

		for (i = 0; i < mParticleEffect->getParticlesCount(); i++)
		{
			C_Particle p;
			p.TTL = C_Random::range(mParticleEffect->getMinTimeToLive(), mParticleEffect->getMaxTimeToLive());
			p.velocity = C_Vector3::random(mParticleEffect->getMinVelocity(), mParticleEffect->getMaxVelocity());
			p.startPos = mParticleEffect->getPos();
			p.accel = C_Vector3::random(mParticleEffect->getMinAcceleration(), mParticleEffect->getMaxAcceleration());
			p.rotation = C_Random::range(mParticleEffect->getMinRotation(), mParticleEffect->getMaxRotation());
			p.rotationSpeed = C_Random::range(mParticleEffect->getMinRotationSpeed(), mParticleEffect->getMaxRotationSpeed());

			if (p.TTL > mMaxTTL)
				mMaxTTL = p.TTL;

			switch(mParticleEffect->getParticleShape())
			{
				case C_PARTICLE_SHAPE_CIRCLE:
				{
					ang = C_Random::range(0.0, 6.283185318);
					rad = C_Random::range(0.0, mParticleEffect->getParticleShapeRadius());

					if (mParticleEffect->getEmitFromShell() == true)
						rad = mParticleEffect->getParticleShapeRadius();

					xsp = rad * cos(ang);
					ysp = 0.0;
					zsp = rad * sin(ang);

					p.startPos = C_Vector3(xsp, ysp, zsp);
					break;
				}

				case C_PARTICLE_SHAPE_SPHERE:
				{
					rad = C_Random::range(0.0, mParticleEffect->getParticleShapeRadius());
					phi = C_Random::range(0.0, 6.283185318);
					tht = C_Random::range(0.0, 3.141592659);

					if (mParticleEffect->getEmitFromShell() == true)
						rad = mParticleEffect->getParticleShapeRadius();

					xsp = rad * cos(phi) * sin(tht);
					ysp = rad * sin(phi) * sin(tht);
					zsp = rad * cos(tht);

					p.startPos = C_Vector3(xsp, ysp, zsp);
					break;
				}
			}

			mParticles.push_back(p);
		}

		mShader = new C_Shader();
		mShader->load("Data/Shaders/particle.vert", "Data/Shaders/particle.frag");
		
		mShader->addAttribute("aPos", 0);
		mShader->addAttribute("aUV", 1);
		mShader->addAttribute("aNorm", 2);
		mShader->addAttribute("aTang", 3);
		mShader->addAttribute("aBitang", 4);
		mShader->compile();

		mBuf = new C_Buffer(vrts, sizeof(vrts) * sizeof(float), 3);
		mTBuf = new C_Buffer(uvs, sizeof(uvs) * sizeof(float), 2);
		mPBuf = new C_Buffer();
		mLBuf = new C_Buffer();
	}
	//////////////////////////////////////////////////////////////////////////////
	//Set particle effect
	void C_ParticleEmitter::setParticleEffect(const C_ParticleEffect* aParticleEffect)
	{
		mParticleEffect = const_cast<C_ParticleEffect*>(aParticleEffect);
	}
	//////////////////////////////////////////////////////////////////////////////
	//Return particle effect
	C_ParticleEffect* C_ParticleEmitter::getParticleEffect() const
	{
		return mParticleEffect;
	}
	//////////////////////////////////////////////////////////////////////////////
	//Set camera pos
	void C_ParticleEmitter::setCameraPos(C_Vector3 aC)
	{
		mCameraPos = aC;
	}
	//////////////////////////////////////////////////////////////////////////////
	//Sort particles
	void C_ParticleEmitter::sort()
	{
		auto func = [](const C_Particle &a, const C_Particle &b) -> bool
		{
			return a.cameraDistance > b.cameraDistance;
		};

		std::sort(mActiveParticles.begin(), mActiveParticles.end(), func);
	}
	//////////////////////////////////////////////////////////////////////////////
	void C_ParticleEmitter::copyActive()
	{
		mActiveParticles.resize(mParticles.size());

		auto copyFunc = [](C_Particle& p)->bool
		{
			return p.active == true;
		};

		auto it = std::copy_if(mParticles.begin(), mParticles.end(), mActiveParticles.begin(), copyFunc);
		mActiveParticles.resize(std::distance(mActiveParticles.begin(), it));
	}
	//////////////////////////////////////////////////////////////////////////////
	//Set light casters, which calculate to using in shaders
	void C_ParticleEmitter::setLights(std::vector<C_Light*> aLights)
	{
		mLights = aLights;
	}
	//////////////////////////////////////////////////////////////////////////////
	void C_ParticleEmitter::update(const float aTimeTick)
	{
		using namespace std;
		
		copyActive();

		float transformation = mParticleEffect->getTransformation();
		C_Vector3 constForce = mParticleEffect->getConstantForce();
		C_Vector3 startEmitterPos = mParticleEffect->getPos();

		float rate = mParticleEffect->getEmitRate();
		float count = mParticleEffect->getParticlesCount();
		float fireT = 1.0 / rate;
		float spawnT = count * fireT;

		if (count <= rate * mMaxTTL)
			spawnT = mMaxTTL;

		float a = mLife;
		float e, life, age;
		bool prevActive;
		C_Vector3 pos, vel, acc;
		float noise;
		float noiseStrength = mParticleEffect->getNoiseStrength();

		size_t counter = 0;

		for (auto& Particle : mParticles)
		{
			e = min(Particle.TTL, fireT) * counter;
			Particle.age = fmod(e + a, spawnT);

			if (transformation == C_PARTICLE_TRANSFORMATION_LOCAL)
				Particle.startEmitterPos = startEmitterPos;
			else
				if ((Particle.age / Particle.TTL) <= aTimeTick)
					Particle.startEmitterPos = startEmitterPos;

			prevActive = Particle.active;
			Particle.active = (Particle.age <= Particle.TTL);

			if (Particle.active == true && prevActive == false)
			{
				Particle.TTL = C_Random::range(mParticleEffect->getMinTimeToLive(), mParticleEffect->getMaxTimeToLive());
				Particle.velocity = C_Vector3::random(mParticleEffect->getMinVelocity(), mParticleEffect->getMaxVelocity());
				Particle.rotationSpeed = C_Random::range(mParticleEffect->getMinRotationSpeed(), mParticleEffect->getMaxRotationSpeed());
			}

			if (Particle.active == true && Particle.age > 0)
			{
				life = fmod(Particle.age, Particle.TTL);

				vel = Particle.velocity;
				acc = Particle.accel;

				age = Particle.age;
				pos = (vel + constForce) * age + (acc * 0.5 * age * age);

				noise = static_cast<float>(mNoise.noise(pos.x, pos.y, pos.z));

				pos += C_Vector3(noise, noise, noise) * noiseStrength;
				pos += Particle.startPos + Particle.startEmitterPos;

				Particle.pos = pos;
				Particle.velocity = vel;
				Particle.rotation += Particle.rotationSpeed * aTimeTick;

				Particle.cameraDistance = pow(mCameraPos.x - Particle.pos.x, 2) + pow(mCameraPos.y - Particle.pos.y, 2) + pow(mCameraPos.z - Particle.pos.z, 2);
			}

			counter++;
		}

		if (mParticleEffect->getSortMode() == C_PARTICLE_SORT_MODE_DISTANCE)
			sort();

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
		C_VertexAttribPointerOpenGL(3, 3, C_OGL_FLOAT, C_OGL_FALSE, 3 * sizeof(float), NULL);
		C_OpenStreamOpenGL(3);
	}
	//////////////////////////////////////////////////////////////////////////////
	void C_ParticleEmitter::setUniforms()
	{
		mShader->setUniform3f("uPos", C_Vector3(0, 0, 0));
		mShader->setUniform2f("uSize", mParticleEffect->getParticleSize());
		mShader->setUniform2f("uStartSize", mParticleEffect->getStartSize());
		mShader->setUniform2f("uFinalSize", mParticleEffect->getFinalSize());
		mShader->setUniform4f("uStartColor", mParticleEffect->getStartColor());
		mShader->setUniform4f("uFinalColor", mParticleEffect->getFinalColor());
		mShader->setUniform1f("uScaleOL", static_cast<float>(mParticleEffect->getScaleOverLifetime()));
		mShader->setUniform1f("uBillboard", static_cast<float>(mParticleEffect->getBillbiarding()));
		mShader->setUniform1f("uGradient", static_cast<float>(mParticleEffect->getGradienting()));

		mShader->setUniformMatrix("uView", C_GetViewMatrix().elements());
		mShader->setUniformMatrix("uProjection", C_GetProjectionMatrix().elements());

		if (mParticleEffect->getMaterial() == nullptr)
			mShader->setUniform4f("uColor", C_Vector4(1, 1, 1, 1));
		else
			mShader->setUniform4f("uColor", mParticleEffect->getMaterial()->getColor());

		if (mParticleEffect->getMaterial() != nullptr)
		{
			C_ActiveTextureOpenGL(C_OGL_TEXTURE0);
			C_BindTextureOpenGL(C_OGL_TEXTURE0, 0);

			if (mParticleEffect->getMaterial()->getTexture() != nullptr)
			{
				mShader->setUniform1i("uTex", 0);
				mParticleEffect->getMaterial()->getTexture()->sampler2D(0);
			}

			mShader->setUniform1i("uDiscard", mParticleEffect->getMaterial()->getDiscard());
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

		float const MaterialUnif[14] =
		{
			matcol.x, matcol.y, matcol.z, matcol.w,
			matamb.x, matamb.y, matamb.z,
			matdif.x, matdif.y, matdif.z,
			matspc.x, matspc.y, matspc.z,
			mParticleEffect->getMaterial()->getReflectionPower()
		};

		mShader->setUniformArrayf("MaterialUnif", MaterialUnif, 14);
	}
	//////////////////////////////////////////////////////////////////////////////
	void C_ParticleEmitter::setShaderLightAndCamera()
	{
		calculateLights();
		mShader->setUniformArrayf("LightUnif", mLightUniform, 120);
	}
	//////////////////////////////////////////////////////////////////////////////
	void C_ParticleEmitter::calculateLights()
	{
		sortLights();
		int i, j, offset;
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
				mLightUniform[9 + offset] = mLights[i]->getType();
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

		C_Shader::unbind();
		C_Texture::unbind();
		C_Buffer::unbind();

		C_CloseStreamOpenGL(0);
		C_CloseStreamOpenGL(1);
		C_CloseStreamOpenGL(2);
		C_CloseStreamOpenGL(3);
	}
	//////////////////////////////////////////////////////////////////////////////
	//Draw particles
	void C_ParticleEmitter::draw()
	{
		if (mParticleEffect == nullptr)
			return;
		if (mShader == nullptr)
			return;
		if (mBuf == nullptr)
			return;
		if (mTBuf == nullptr)
			return;
		if (mParticleEffect->getVisible() == false)
			return;

		mShader->bind();

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
			delete mPosData;
			delete mTimeData;

			mParticlesCount = mParticleEffect->getParticlesCount();

			mVertData = new float[mParticlesCount * 18];
			mUvData = new float[mParticlesCount * 12];
			mPosData = new float[mParticlesCount * 18];
			mTimeData = new float[mParticlesCount * 18];

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
			}
		}

		mBuf->setData(mVertData, 18 * sizeof(float) * mActiveParticles.size(), 3);
		mBuf->compile();

		mTBuf->setData(mUvData, 12 * sizeof(float) * mActiveParticles.size(), 2);
		mTBuf->compile();

		mPBuf->setData(mPosData, 18 * sizeof(float)* mActiveParticles.size(), 3);
		mPBuf->compile();

		mLBuf->setData(mTimeData, 18 * sizeof(float) * mActiveParticles.size(), 3);
		mLBuf->compile();

		setBuffers();

		C_DrawArraysOpenGL(C_OGL_TRIANGLES, 0, 6 * mActiveParticles.size());

		unbindAll();
	}
	//////////////////////////////////////////////////////////////////////////////
	//Destructor
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
