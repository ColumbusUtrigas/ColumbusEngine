/************************************************
*              ParticleEmitter.cpp              *
*************************************************
*          This file is a part of:              *
*               COLUMBUS ENGINE                 *
*************************************************
*             Nikolay(Columbus) Red             *
*                   20.07.2017                  *
*************************************************/

#include <Graphics/ParticleEmitter.h>

namespace C
{

	//////////////////////////////////////////////////////////////////////////////
	//Constructor
	C_ParticleEmitter::C_ParticleEmitter(const C_ParticleEffect* aParticleEffect) :
	mParticleEffect((C_ParticleEffect*)aParticleEffect)
	{
		if (aParticleEffect == nullptr)
			return;

		for (int i = 0; i < mParticleEffect->getParticlesCount(); i++)
		{
			C_Particle p;
			p.TTL = C_RandomBetween(mParticleEffect->getMinTimeToLive(), mParticleEffect->getMaxTimeToLive());
			p.velocity = C_RandomBetween(mParticleEffect->getMinVelocity(), mParticleEffect->getMaxVelocity());
			p.direction = C_Vector3::random(mParticleEffect->getMinDirection(), mParticleEffect->getMaxDirection());
			p.accel = C_Vector3::random(mParticleEffect->getMinAcceleration(), mParticleEffect->getMaxAcceleration());

			if (p.TTL > mMaxTTL)
				mMaxTTL = p.TTL;

			switch(mParticleEffect->getParticleShape())
			{
				case C_PARTICLE_SHAPE_CIRCLE:
				{
					float ang = C_RandomBetween(0.0, 6.283185318);
					float rad = C_RandomBetween(0.0, mParticleEffect->getParticleShapeRadius());

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
					float rad = C_RandomBetween(0.0, mParticleEffect->getParticleShapeRadius());
					float phi = C_RandomBetween(0.0, 6.283185318);
					float tht = C_RandomBetween(0.0, 3.141592659);

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

		mBuf = new C_Buffer(vrts, sizeof(vrts) * sizeof(float));
		mTBuf = new C_Buffer(uvs, sizeof(uvs) * sizeof(float));
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
		float mCX = mCameraPos.x;
		float mCY = mCameraPos.y;
		float mCZ = mCameraPos.z;

		auto func = [mCX, mCY, mCZ](const C_Particle &a, const C_Particle &b) -> bool
		{
			float l1 = (a.pos.x - mCX) * (a.pos.x - mCX) + (a.pos.y - mCY) * (a.pos.y - mCY) + (a.pos.z - mCZ) * (a.pos.z - mCZ);
			float l2 = (b.pos.x - mCX) * (b.pos.x - mCX) + (b.pos.y - mCY) * (b.pos.y - mCY) + (b.pos.z - mCZ) * (b.pos.z - mCZ);

			return l1 > l2;

			/*glm::vec3 mC(mCX, mCY, mCZ);

			glm::vec3 q(a.pos.x, a.pos.y, a.pos.z);
			glm::vec3 w(b.pos.x, b.pos.y, b.pos.z);

			q -= mC;
			w -= mC;

			return glm::length(q) > glm::length(w);*/
		};

		std::sort(mParticles.begin(), mParticles.end(), func);
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

		float a = tm.elapsed();

		mBuf->bind();
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(0);
		mTBuf->bind();
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(1);

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
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, 0);

			if (mParticleEffect->getMaterial()->getTexture() != nullptr)
			{
				mShader->setUniform1i("uTex", 0);
				mParticleEffect->getMaterial()->getTexture()->sampler2D(0);
			}

			mShader->setUniform1i("uDiscard", mParticleEffect->getMaterial()->getDiscard());
		}

		if (mFrame >= 10)
		{
			//sort();
			mFrame = 0;
		}

		mFrame++;

		glDepthMask(GL_FALSE);

		if (mParticleEffect->getAdditive())
			glBlendFunc(GL_SRC_ALPHA, GL_ONE);

		float scaleOL = mParticleEffect->getScaleOverLifetime();
		float billboard = mParticleEffect->getBillbiarding();
		float gradient = mParticleEffect->getGradienting();
		C_Vector3 constForce = mParticleEffect->getConstantForce();

		float rate = mParticleEffect->getEmitRate();
		float count = mParticleEffect->getParticlesCount();
		float fireT = 1.0 / rate;
		float spawnT = count * fireT;

		if (count <= rate * mMaxTTL)
			spawnT = mMaxTTL;

		a = fmod(a, spawnT);

		for (int i = 0; i < mParticleEffect->getParticlesCount(); i++)
		{
			//float e = mParticles[i].TTL / mParticleEffect->getParticlesCount();
<<<<<<< HEAD
			float e = min(mParticles[i].TTL, fireT) * i;
=======
			float e = std::min(mParticles[i].TTL, fireT) * i;
>>>>>>> dev

			mParticles[i].age = fmod(e + a, spawnT);
			mParticles[i].active = (mParticles[i].age <= mParticles[i].TTL);


			if (mParticles[i].active == true && mParticles[i].age > 0)
			{
				float life = fmod(mParticles[i].age, mParticles[i].TTL);

				C_Vector3 vel = mParticles[i].direction.normalize() * mParticles[i].velocity;
				C_Vector3 acc = mParticles[i].accel;

				float age = mParticles[i].age;

				C_Vector3 pos = (vel + constForce) * age + (acc * 0.5 * age * age);
				pos += mParticles[i].startPos;
				mParticles[i].pos = pos;

				float arr[8] = {pos.x, pos.y, pos.z, life, mParticles[i].TTL, scaleOL, billboard, gradient};

				//mShader->setUniform4f("uPosition", set);
				mShader->setUniformArrayf("Unif", arr, 8);

				glDrawArrays(GL_TRIANGLES, 0, 6);
			}

			//mParticles[i].age += frame.elapsed();
		}

		if (mLife >= mMaxTTL)
			mLife = 0.0;
		mLife += frame.elapsed();

		frame.reset();

		if (mParticleEffect->getAdditive())
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glDepthMask(GL_TRUE);

		C_Shader::unbind();
		C_Texture::unbind();
		C_Buffer::unbind();

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
	}
	//////////////////////////////////////////////////////////////////////////////
	//Destructor
	C_ParticleEmitter::~C_ParticleEmitter()
	{
		if (mBuf != nullptr)
			delete mBuf;
		if (mShader != nullptr)
			delete mShader;

		mParticles.clear();
	}

}
