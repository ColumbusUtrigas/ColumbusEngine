#include <Graphics/ParticleEmitter.h>

namespace C
{

	C_ParticleEmitter::C_ParticleEmitter(size_t aSize) : mSize(aSize)
	{
		C_Particle p;
	
		for (size_t i = 0; i < aSize; i++)
		{
			mParticles.push_back(p);
		}
		
		mShader = new C_Shader("Data/Shaders/particle.vert", "Data/Shaders/particle.frag");
		
		float vrts[] = 
		{
			0.1, 0.1, 0.0,
			-0.1, 0.1, 0.0,
			-0.1, -0.1, 0.0,
			-0.1, -0.1, 0.0,
			0.1, -0.1, 0.0,
			0.1, 0.1, 0.0
		};
		
		mBuf = new C_Buffer(vrts, sizeof(vrts) * sizeof(float));
		
		float uvs[] = 
		{
			1, 1,
			0, 1,
			0, 0,
			0, 0,
			1, 0,
			1, 1
		};
		
		mTBuf = new C_Buffer(uvs, sizeof(uvs) * sizeof(float));
	}
	
	void C_ParticleEmitter::draw()
	{
		glDisable(GL_CULL_FACE);
	
		for (size_t i = 0; i < mSize; i++)
		{
			if (mParticles[i].age < mParticles[i].lifetime)
				mParticles[i].age = mParticles[i].tm.elapsed() / 1000000;
			else
				mParticles[i].reset();
				
			if (mBuf == nullptr)
				return;
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
				return;
			}
			
			mShader->bind();
			
			mShader->setUniform3f("uPos", mParticles[i].pos);
			mShader->setUniform3f("uVel", mParticles[i].speed);
			mShader->setUniform3f("uAcc", mParticles[i].vel);
			mShader->setUniform1f("uTTL", mParticles[i].lifetime);
			mShader->setUniform1f("uTime", mParticles[i].age);
			mShader->setUniformMatrix("uView", glm::value_ptr(C_GetViewMatrix()));
			mShader->setUniformMatrix("uProjection", glm::value_ptr(C_GetProjectionMatrix()));
			
			glDrawArrays(GL_TRIANGLES, 0, 6);
			
			C_Shader::unbind();
				
			glDisableVertexAttribArray(0);
			glDisableVertexAttribArray(1);
		}
		
		glEnable(GL_CULL_FACE);
	}
	
	C_ParticleEmitter::~C_ParticleEmitter()
	{
		if (mBuf != nullptr)
			delete mBuf;
			
		if (mTBuf != nullptr)
			delete mTBuf;
	}

}
















