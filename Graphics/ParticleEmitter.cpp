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
			p.direction = C_Vector3::random(C_Vector3(-1, -1, -1), C_Vector3(1, 1, 1));

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
		if (mParticleEffect->getVisible() == false)
			return;

		float e = mParticles[0].TTL / mParticleEffect->getParticlesCount();
		float a = tm.elapsed();

		mBuf->bind();
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(0);

		C_Texture::unbind();

		mShader->bind();

		mShader->setUniform3f("uPos", C_Vector3(0, 0, 0));

		mShader->setUniformMatrix("uView", glm::value_ptr(C_GetViewMatrix()));
		mShader->setUniformMatrix("uProjection", glm::value_ptr(C_GetProjectionMatrix()));

		if (mParticleEffect->getMaterial() == nullptr)
			mShader->setUniform4f("uColor", C_Vector4(1, 1, 1, 1));
		else
			mShader->setUniform4f("uColor", mParticleEffect->getMaterial()->getColor());

		for (int i = 0; i < mParticleEffect->getParticlesCount(); i++)
		{
			if (a >= (e * i))
				mParticles[i].active = true;
			else
				mParticles[i].tm.reset();


			if (mParticles[i].active == true)
			{
				float life = (int)(mParticles[i].tm.elapsed() * 1000) % (int)(mParticles[i].TTL * 1000);

				mShader->setUniform1f("uTime", (float)life / 1000);
				mShader->setUniform3f("uDirection", mParticles[i].direction);
				mShader->setUniform1f("uVel", mParticles[i].velocity);
				mShader->setUniform1f("uAcc", 0.0);
				glDrawArrays(GL_TRIANGLES, 0, 6);
			}
		}

		C_Shader::unbind();
		C_Texture::unbind();
		C_Buffer::unbind();

		glDisableVertexAttribArray(0);
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
