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

#include <System/System.h>
#include <System/Timer.h>
#include <Math/Vector2.h>
#include <Math/Vector3.h>
#include <Math/Vector4.h>
#include <Graphics/Buffer.h>
#include <Graphics/Camera.h>
#include <Graphics/ParticleEffect.h>

namespace C
{

	struct C_Particle
	{
		C_Vector3 velocity = C_Vector3(0.0, 1.0, 0.0);

		C_Timer tm;

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

		C_Timer tm;

		//Vertex buffer
		float vrts[18] =
		{
			0.1, 0.1, 0.0,
			-0.1, 0.1, 0.0,
			-0.1, -0.1, 0.0,
			-0.1, -0.1, 0.0,
			0.1, -0.1, 0.0,
			0.1, 0.1, 0.0
		};
	public:
		//Constructor
		C_ParticleEmitter(const C_ParticleEffect* aParticleEffect);
		//Set particle effect
		void setParticleEffect(const C_ParticleEffect* aParticleEffect);
		//Draw particles
		void draw();
		//Destructor
		~C_ParticleEmitter();
	};

}
