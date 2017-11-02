/************************************************
*              ParticleEmitter.h                *
*************************************************
*          This file is a part of:              *
*               COLUMBUS ENGINE                 *
*************************************************
*             Nikolay(Columbus) Red             *
*                   20.07.2017                  *
*************************************************/

#pragma once

#include <cstdio>
#include <cstdlib>
#include <vector>
#include <ctime>
#include <sstream>
#include <algorithm>

#include <glm/glm.hpp>

#include <RenderAPI/Buffer.h>

#include <Math/Vector2.h>
#include <Math/Vector3.h>
#include <Math/Vector4.h>
#include <Graphics/Camera.h>
#include <Graphics/ParticleEffect.h>
#include <System/System.h>
#include <System/Timer.h>
#include <System/Random.h>

namespace Columbus
{

	struct C_Particle
	{
		C_Vector3 direction = C_Vector3(0, 1, 0);

		C_Vector3 startPos = C_Vector3(0, 0, 0);

		C_Vector3 startEmitterPos = C_Vector3(0, 0, 0);

		C_Vector3 accel = C_Vector3(0, 0.1, 0);

		C_Vector3 pos;

		float velocity = 1.0;

		float age = 0.0;

		float TTL = 1.0;

		bool active = false;
	};

	class C_ParticleEmitter
	{
	private:
		C_ParticleEffect* mParticleEffect = NULL;

		std::vector<C_Particle> mParticles;

		C_Shader* mShader = NULL;

		C_Buffer* mBuf = NULL;
		C_Buffer* mTBuf = NULL;

		int mFrame = 0;

		float mLife = 0.0;

		float mMaxTTL = 0.0;

		C_Vector3 mCameraPos = C_Vector3(0, 0, 5);

		//Vertex buffer
		float vrts[18] =
		{
			1, 1, 0.0,
			-1, 1, 0.0,
			-1, -1, 0.0,
			-1, -1, 0.0,
			1, -1, 0.0,
			1, 1, 0.0
		};
		//UV buffer
		float uvs[12] =
		{
			1.0, 1.0,
			0.0, 1.0,
			0.0, 0.0,
			0.0, 0.0,
			1.0, 0.0,
			1.0, 1.0
		};
		void sort();
		void update(float aTimeTick);
		void setBuffers();
		void setUniforms();
		void unbindAll();

	public:
		//Constructor
		C_ParticleEmitter(const C_ParticleEffect* aParticleEffect);
		//Set particle effect
		void setParticleEffect(const C_ParticleEffect* aParticleEffect);
		//Set camera pos
		void setCameraPos(C_Vector3 aC);
		//Draw particles
		//void draw();
		void draw(float aTimeTick);
		//Destructor
		~C_ParticleEmitter();
	};

}
