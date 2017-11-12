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

		for (int i = 0; i < mParticleEffect->getParticlesCount(); i++)
		{
			C_Particle p;
			p.TTL = C_Random::range(mParticleEffect->getMinTimeToLive(), mParticleEffect->getMaxTimeToLive());
			p.velocity = C_Random::range(mParticleEffect->getMinVelocity(), mParticleEffect->getMaxVelocity());
			p.startPos = mParticleEffect->getPos();
			p.direction = C_Vector3::random(mParticleEffect->getMinDirection(), mParticleEffect->getMaxDirection());
			p.accel = C_Vector3::random(mParticleEffect->getMinAcceleration(), mParticleEffect->getMaxAcceleration());
			p.rotation = C_Random::range(mParticleEffect->getMinRotation(), mParticleEffect->getMaxRotation());
			p.rotationSpeed = C_Random::range(mParticleEffect->getMinRotationSpeed(), mParticleEffect->getMaxRotationSpeed());

			if (p.TTL > mMaxTTL)
				mMaxTTL = p.TTL;

			switch(mParticleEffect->getParticleShape())
			{
				case C_PARTICLE_SHAPE_CIRCLE:
				{
					float ang = C_Random::range(0.0, 6.283185318);
					float rad = C_Random::range(0.0, mParticleEffect->getParticleShapeRadius());

					if (mParticleEffect->getEmitFromShell() == true)
						rad = mParticleEffect->getParticleShapeRadius();

					float xsp = rad * cos(ang);
					float ysp = 0.0;
					float zsp = rad * sin(ang);

					p.startPos = C_Vector3(xsp, ysp, zsp);
					break;
				}

				case C_PARTICLE_SHAPE_SPHERE:
				{
					float rad = C_Random::range(0.0, mParticleEffect->getParticleShapeRadius());
					float phi = C_Random::range(0.0, 6.283185318);
					float tht = C_Random::range(0.0, 3.141592659);

					if (mParticleEffect->getEmitFromShell() == true)
						rad = mParticleEffect->getParticleShapeRadius();

					float xsp = rad * cos(phi) * sin(tht);
					float ysp = rad * sin(phi) * sin(tht);
					float zsp = rad * cos(tht);

					p.startPos = C_Vector3(xsp, ysp, zsp);
					break;
				}
			}

			mParticles.push_back(p);
		}

		mShader = new C_Shader("Data/Shaders/particle.vert", "Data/Shaders/particle.frag");

		mBuf = new C_Buffer(vrts, sizeof(vrts) * sizeof(float), 3);
		mTBuf = new C_Buffer(uvs, sizeof(uvs) * sizeof(float), 2);
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
		C_Vector3 pos = mCameraPos;

		auto func = [pos](const C_Particle &a, const C_Particle &b) -> bool
		{
			C_Vector3 q = a.pos;
			C_Vector3 w = b.pos;

			return q.length(pos) > w.length(pos);
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
	void C_ParticleEmitter::update(float aTimeTick)
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

		int counter = 0;

		for (auto& Particle : mParticles)
		{
			float e = min(Particle.TTL, fireT) * counter;
			Particle.age = fmod(e + a, spawnT);

			if (transformation == C_PARTICLE_TRANSFORMATION_LOCAL)
				Particle.startEmitterPos = startEmitterPos;
			else
				if ((Particle.age / Particle.TTL) <= aTimeTick)
					Particle.startEmitterPos = startEmitterPos;

			Particle.active = (Particle.age <= Particle.TTL);


			if (Particle.active == true && Particle.age > 0)
			{
				float life = fmod(Particle.age, Particle.TTL);

				C_Vector3 vel = Particle.direction.normalize() * Particle.velocity;
				C_Vector3 acc = Particle.accel;

				float age = Particle.age;

				C_Vector3 pos = (vel + constForce) * age + (acc * 0.5 * age * age);
				pos += Particle.startPos + Particle.startEmitterPos;
				Particle.pos = pos;
				Particle.rotation += Particle.rotationSpeed * aTimeTick;
			}

			counter++;
		}

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
	}
	//////////////////////////////////////////////////////////////////////////////
	//Draw particles
	/*void C_ParticleEmitter::draw()
	{
		using namespace std;

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

		float a = tm.elapsed();

		mBuf->bind();
		C_VertexAttribPointerOpenGL(0, 3, C_OGL_FLOAT, C_OGL_FALSE, 3 * sizeof(float), NULL);
		C_OpenStreamOpenGL(0);
		mTBuf->bind();
		C_VertexAttribPointerOpenGL(1, 2, C_OGL_FLOAT, C_OGL_FALSE, 2 * sizeof(float), NULL);
		C_OpenStreamOpenGL(1);

		mShader->bind();

		mShader->setUniform3f("uPos", C_Vector3(0, 0, 0));
		mShader->setUniform2f("uSize", mParticleEffect->getParticleSize());
		mShader->setUniform2f("uStartSize", mParticleEffect->getStartSize());
		mShader->setUniform2f("uFinalSize", mParticleEffect->getFinalSize());
		mShader->setUniform4f("uStartColor", mParticleEffect->getStartColor());
		mShader->setUniform4f("uFinalColor", mParticleEffect->getFinalColor());

		mShader->setUniformMatrix("uView", glm::value_ptr(C_GetViewMatrix()));
		mShader->setUniformMatrix("uProjection", glm::value_ptr(C_GetProjectionMatrix()));

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

		C_DisableDepthMaskOpenGL();

		if (mParticleEffect->getAdditive())
			C_BlendFuncOpenGL(C_OGL_SRC_ALPHA, C_OGL_ONE);

		float scaleOL = mParticleEffect->getScaleOverLifetime();
		float billboard = mParticleEffect->getBillbiarding();
		float gradient = mParticleEffect->getGradienting();

		float transformation = mParticleEffect->getTransformation();
		C_Vector3 constForce = mParticleEffect->getConstantForce();
		C_Vector3 startEmitterPos = mParticleEffect->getPos();

		float rate = mParticleEffect->getEmitRate();
		float count = mParticleEffect->getParticlesCount();
		float fireT = 1.0 / rate;
		float spawnT = count * fireT;

		if (count <= rate * mMaxTTL)
			spawnT = mMaxTTL;

		a = fmod(mLife, spawnT);

		for (int i = 0; i < mParticleEffect->getParticlesCount(); i++)
		{
			float e = min(mParticles[i].TTL, fireT) * i;
			mParticles[i].age = fmod(e + a, spawnT);

			if (transformation == C_PARTICLE_TRANSFORMATION_LOCAL)
				mParticles[i].startEmitterPos = startEmitterPos;
			else
				if ((mParticles[i].age / mParticles[i].TTL) <= 0.1)
					mParticles[i].startEmitterPos = startEmitterPos;

			mParticles[i].active = (mParticles[i].age <= mParticles[i].TTL);


			if (mParticles[i].active == true && mParticles[i].age > 0)
			{
				float life = fmod(mParticles[i].age, mParticles[i].TTL);

				C_Vector3 vel = mParticles[i].direction.normalize() * mParticles[i].velocity;
				C_Vector3 acc = mParticles[i].accel;

				float age = mParticles[i].age;

				C_Vector3 pos = (vel + constForce) * age + (acc * 0.5 * age * age);
				pos += mParticles[i].startPos + mParticles[i].startEmitterPos;
				mParticles[i].pos = pos;

				float arr[8] = {pos.x, pos.y, pos.z, life, mParticles[i].TTL, scaleOL, billboard, gradient};

				mShader->setUniformArrayf("Unif", arr, 8);

				C_DrawArraysOpenGL(C_OGL_TRIANGLES, 0, 6);
			}
		}

		if (mLife >= mMaxTTL)
			mLife = 0.0;
		mLife += frame.elapsed();

		frame.reset();

		if (mParticleEffect->getAdditive())
			C_BlendFuncOpenGL(C_OGL_SRC_ALPHA, C_OGL_ONE_MINUS_SRC_ALPHA);

		C_EnableDepthMaskOpenGL();

		C_Shader::unbind();
		C_Texture::unbind();
		C_Buffer::unbind();

		C_CloseStreamOpenGL(0);
		C_CloseStreamOpenGL(1);
	}*/
	//////////////////////////////////////////////////////////////////////////////
	//Draw particles
	void C_ParticleEmitter::draw(float aTimeTick)
	{
		update(aTimeTick);

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

		setBuffers();

		mShader->bind();

		setUniforms();

		C_DisableDepthMaskOpenGL();

		if (mParticleEffect->getAdditive())
			C_BlendFuncOpenGL(C_OGL_SRC_ALPHA, C_OGL_ONE);

		float scaleOL = mParticleEffect->getScaleOverLifetime();
		float billboard = mParticleEffect->getBillbiarding();
		float gradient = mParticleEffect->getGradienting();

		for (auto Particle : mActiveParticles)
		{
			if (Particle.active == true && Particle.age > 0)
			{
				float life = fmod(Particle.age, Particle.TTL);
				C_Vector3 pos = Particle.pos;
				float rotation = Particle.rotation;

				float arr[9] = { pos.x, pos.y, pos.z, life, Particle.TTL, scaleOL, billboard, gradient, rotation};

				mShader->setUniformArrayf("Unif", arr, 9);

				C_DrawArraysOpenGL(C_OGL_TRIANGLES, 0, 6);
			}
		}

		unbindAll();
	}
	//////////////////////////////////////////////////////////////////////////////
	//Destructor
	C_ParticleEmitter::~C_ParticleEmitter()
	{
		mParticles.clear();
	}

}
