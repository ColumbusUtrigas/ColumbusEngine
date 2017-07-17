#pragma once

#include <cstdio>
#include <cstdlib>
#include <vector>
#include <ctime>

#include <System/System.h>
#include <System/Timer.h>
#include <Math/Vector2.h>
#include <Math/Vector3.h>
#include <Math/Vector4.h>
#include <Graphics/Shader.h>
#include <Graphics/Buffer.h>
#include <Graphics/Camera.h>
#include <Graphics/Texture.h>

namespace C
{

	struct C_Particle
	{
		C_Vector3 random;

		C_Vector3 pos = C_Vector3(0, 0, 0);
		float vel = 0.0;
		float speed = 1.0;
		C_Timer tm;

		float lifetime = 1.0;
		float age = 0;

		bool active = false;

		inline C_Particle(C_Vector3 aPos, float aVel, float aSpd, float aTTL)
		{
			pos = aPos;
			vel = aVel;
			speed = aSpd;
			lifetime = aTTL;

			random = C_Vector3::random(C_Vector3(-1, -1, -1), C_Vector3(1, 1, 1));
		}

		inline void reset(C_Vector3 aPos, float aVel, float aSpd, float aTTL)
		{
			pos = aPos;
			vel = aVel;
			speed = aSpd;
			lifetime = aTTL;

			age = 0;

			tm.reset();

			active = false;
		}

		inline void reset(C_Vector3 aPos, float aVel, float aSpd, float aTTL, C_Vector3 aRandom)
		{
			pos = aPos;
			vel = aVel;
			speed = aSpd;
			lifetime = aTTL;

			age = 0;

			random = C_Vector3::random(C_Vector3(0, 0, 0) - aRandom, aRandom);

			tm.reset();

			active = false;
		}

		inline void reset(C_Vector3 aPos, float aVel, float aSpd, float aTTL, C_Vector3 aRandom1, C_Vector3 aRandom2)
		{
			pos = aPos;
			vel = aVel;
			speed = aSpd;
			lifetime = aTTL;

			age = 0;

			random = C_Vector3::random(aRandom1, aRandom2);

			tm.reset();

			active = false;
		}

		inline ~C_Particle() {}
	};

	enum C_ParticleSystemRenderModes
	{
		C_PARTICLE_SYSTEM_RENDER_MODE_BILLBOARD,
		C_PARTICLE_SYSTEM_RENDER_MODE_STATIC,
	};

	class C_ParticleEmitter
	{
	private:
		std::vector<C_Particle> mParticles;

		size_t mCount = 100;

		C_Shader* mShader = NULL;

		C_Buffer* mBuf = NULL;
		C_Buffer* mTBuf = NULL;

		C_Timer tm;

		C_Vector3 mPos = C_Vector3(0, 0, 0);
		C_Vector4 mColor = C_Vector4(1, 1, 1, 1);
		C_Vector3 mRandom1 = C_Vector3(1, 1, 1);
		C_Vector3 mRandom2 = C_Vector3(-1, -1, -1);

		C_Texture* mTexture = NULL;

		float mAcc = 0.0;
		float mSpeed = 1.0;
		float mLifetime = 1.0;
		float mSize = 1.0;

		int mRenderMode = C_PARTICLE_SYSTEM_RENDER_MODE_BILLBOARD;

		float vrts[18] =
		{
			0.1, 0.1, 0.0,
			-0.1, 0.1, 0.0,
			-0.1, -0.1, 0.0,
			-0.1, -0.1, 0.0,
			0.1, -0.1, 0.0,
			0.1, 0.1, 0.0
		};

		float uvs[12] =
		{
			1, 1,
			0, 1,
			0, 0,
			0, 0,
			1, 0,
			1, 1
		};
	public:
		C_ParticleEmitter(size_t aCount = 100, C_Vector3 aPos = C_Vector3(0, 0, 0), float aAcc = 0.0, float aSpd = 1.0, float aTTL = 1.0);

		void draw();

		inline void setPos(const C_Vector3 aPos = C_Vector3(0, 0, 0)) { mPos = (C_Vector3)aPos; }

		inline void setAcc(const float aAcc = 0.0) { mAcc = (float)aAcc; }

		inline void setSpeed(const float aSpeed = 1.0) { mSpeed = (float)aSpeed; }

		inline void setLifetime(const float aTTL = 1.0) { mLifetime = (float)aTTL; }

		inline void setSize(const float aSize = 1.0) { mSize = (float)aSize; }

		inline void setTexture(const C_Texture* aTexture) { mTexture = (C_Texture*)aTexture; }

		inline void setRenderMode(C_ParticleSystemRenderModes aX) { mRenderMode = aX; }

		void setRandom(const C_Vector3 aRandom);

		void setRandom(const C_Vector3 aRandom1, const C_Vector3 aRandom2);

		inline C_Vector3 getPos() { return mPos; }

		inline float getAcc() { return mAcc; }

		inline float getSpeed() { return mSpeed; }

		inline float getLifetime() { return mLifetime; }

		inline float getSize() { return mSize; }

		inline C_Texture* getTexture() { return mTexture; }

		inline int getRenderMode() { return mRenderMode; }

		~C_ParticleEmitter();
	};

}
