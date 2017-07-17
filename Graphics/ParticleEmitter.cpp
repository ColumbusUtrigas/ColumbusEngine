#include <Graphics/ParticleEmitter.h>

namespace C
{

	C_ParticleEmitter::C_ParticleEmitter(size_t aCount, C_Vector3 aPos, float aAcc, float aSpd, float aTTL)
	: mCount(aCount), mPos(aPos), mAcc(aAcc), mSpeed(aSpd), mLifetime(aTTL)
	{
		for (size_t i = 0; i < aCount; i++)
		{
			C_Particle p(aPos, aAcc, aSpd, aTTL);
			mParticles.push_back(p);
		}
		
		mShader = new C_Shader("Data/Shaders/particle.vert", "Data/Shaders/particle.frag");
		
		mBuf = new C_Buffer(vrts, sizeof(vrts) * sizeof(float));
		
		mTBuf = new C_Buffer(uvs, sizeof(uvs) * sizeof(float));
	}
	
	void C_ParticleEmitter::draw()
	{
		glDisable(GL_CULL_FACE);
		
		float e = 0;
		
		if (mCount != 0)
			e = (float)mParticles[0].lifetime / (float)mCount;
		else
		{
			glEnable(GL_CULL_FACE);
			return;
		}
		
		float a = tm.elapsed() / 1000000;
		
		if (mBuf == nullptr)
		{
			glEnable(GL_CULL_FACE);
			return;
		}
		
		mBuf->bind();
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(0);

		if (mTBuf != nullptr)
		{
			mTBuf->bind();
			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (GLvoid*)0);
			glEnableVertexAttribArray(1);
		}

		if (mShader == nullptr)
		{
			glDisableVertexAttribArray(0);
			glDisableVertexAttribArray(1);
			glEnable(GL_CULL_FACE);
			return;
		}
		
		C_Texture::unbind();

		mShader->bind();
		
		//Vertex Shader Uniforms
		mShader->setUniform3f("uPos", mPos);
		mShader->setUniform1f("uVel", mSpeed);
		mShader->setUniform1f("uAcc", mAcc);
		mShader->setUniform1f("uTTL", mLifetime);
		mShader->setUniform1f("uSize", mSize);
		mShader->setUniform1i("uRenderMode", mRenderMode);
		
		mShader->setUniformMatrix("uView", glm::value_ptr(C_GetViewMatrix()));
		mShader->setUniformMatrix("uProjection", glm::value_ptr(C_GetProjectionMatrix()));
		
		//Fragment Shader Uniforms
		mShader->setUniform4f("uColor", mColor);
		if (mTexture != nullptr)
		{
			mShader->setUniform1i("uTex", 0);
			mTexture->sampler2D(0);
		}
		
		for (size_t i = 0; i < mCount; i++)
		{
			if (a >= (e * i))
				mParticles[i].active = true;
			else
				mParticles[i].reset(mPos, mAcc, mSpeed, mLifetime);
		
			if (mParticles[i].active != false)
			{
				if (mParticles[i].age < mParticles[i].lifetime)
					mParticles[i].age = mParticles[i].tm.elapsed() / 1000000;
				else
					mParticles[i].reset(mPos, mAcc, mSpeed, mLifetime);
			
				mShader->setUniform1f("uTime", mParticles[i].age);
				mShader->setUniform3f("uRandom", mParticles[i].random);
			
				glDrawArrays(GL_TRIANGLES, 0, 6);
			}
			
		}
		
		C_Shader::unbind();
				
		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		
		glEnable(GL_CULL_FACE);
	}
	
	void C_ParticleEmitter::setRandom(const C_Vector3 aRandom)
	{
		mRandom2 = (C_Vector3)aRandom;
		for (size_t i = 0; i < mParticles.size(); i++)
			mParticles[i].reset(mPos, mAcc, mSpeed, mLifetime, mRandom2);
	}
	
	void C_ParticleEmitter::setRandom(const C_Vector3 aRandom1, const C_Vector3 aRandom2)
	{
		mRandom1 = (C_Vector3)aRandom1;
		mRandom2 = (C_Vector3)aRandom2;
		for (size_t i = 0; i < mParticles.size(); i++)
			mParticles[i].reset(mPos, mAcc, mSpeed, mLifetime, mRandom1, mRandom2);
	}
	
	C_ParticleEmitter::~C_ParticleEmitter()
	{
		if (mBuf != nullptr)
			delete mBuf;
			
		if (mTBuf != nullptr)
			delete mTBuf;
	}

}
















