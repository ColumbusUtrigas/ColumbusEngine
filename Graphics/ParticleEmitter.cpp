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

			mParticles.push_back(p);
		}

		mShader = new C_Shader("Data/Shaders/particle.vert", "Data/Shaders/particle.frag");

		mBuf = new C_Buffer(vrts, sizeof(vrts) * sizeof(float));
		mTBuf = new C_Buffer(uvs, sizeof(uvs) * sizeof(float));
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

		float e = mParticles[0].TTL / mParticleEffect->getParticlesCount();
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
		}

		if (mFrame >= 10)
		{
			auto func = [](const C_Particle & a, const C_Particle & b) -> bool
			{
				return a.pos.z < b.pos.z;
			};

			std::sort(mParticles.begin(), mParticles.end(), func);
			mFrame = 0;
		}

		mFrame++;

		for (int i = 0; i < mParticleEffect->getParticlesCount(); i++)
		{
			if (a >= (e * i))
				mParticles[i].active = true;
			else
				mParticles[i].tm.reset();


			if (mParticles[i].active == true)
			{
				float life = (int)(mParticles[i].tm.elapsed() * 1000) % (int)(mParticles[i].TTL * 1000);

				C_Vector3 pos = mParticles[i].direction.normalize() * (float)(life / 1000) * mParticles[i].velocity;

				mParticles[i].pos = pos;

				float arr[6] = {pos.x, pos.y, pos.z, (float)(life / 1000), mParticles[i].TTL, 1.0};

				//mShader->setUniform4f("uPosition", set);
				mShader->setUniformArrayf("Unif", arr, 6);

				glDrawArrays(GL_TRIANGLES, 0, 6);
			}
		}

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
