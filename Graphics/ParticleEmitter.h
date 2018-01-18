/************************************************
*              ParticleEmitter.h                *
*************************************************
*          This file is a part of:              *
*               COLUMBUS ENGINE                 *
*************************************************
*                Nika(Columbus) Red             *
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
#include <Graphics/Light.h>
#include <System/System.h>
#include <System/Timer.h>
#include <System/Random.h>
#include <Common/Noise/PerlinNoise.h>

namespace Columbus
{

	struct C_Particle
	{
		C_Vector3 velocity = C_Vector3(0, 1, 0);
		C_Vector3 startPos = C_Vector3(0, 0, 0);
		C_Vector3 startEmitterPos = C_Vector3(0, 0, 0);
		C_Vector3 accel = C_Vector3(0, 0.1, 0);
		C_Vector3 pos;

		float rotation = 0.0;
		float rotationSpeed = 0.0;
		float age = 0.0;
		float TTL = 1.0;
		float cameraDistance = 1000.0;

		bool active = false;
	};

	class C_ParticleEmitter
	{
	private:
		C_ParticleEffect* mParticleEffect = nullptr;

		std::vector<C_Particle> mParticles;
		std::vector<C_Particle> mActiveParticles;
		std::vector<C_Light*> mLights;

		C_Buffer* mBuf = nullptr;
		C_Buffer* mTBuf = nullptr;
		C_Buffer* mPBuf = nullptr;
		C_Buffer* mLBuf = nullptr;

		C_PerlinNoise mNoise;

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
		void copyActive();
		void setBuffers();
		void setUniforms();
		void setShaderMaterial();
		void setShaderLightAndCamera();
		void calculateLights();
		void sortLights();
		void unbindAll();
		
		float mLightUniform[120];
		float* mVertData = nullptr;
		float* mUvData = nullptr;
		float* mPosData = nullptr;
		float* mTimeData = nullptr;
		unsigned int mParticlesCount = 0;
	public:
		C_Shader* mShader = nullptr;

		//Constructor
		C_ParticleEmitter(const C_ParticleEffect* aParticleEffect);
		//Set particle effect
		void setParticleEffect(const C_ParticleEffect* aParticleEffect);
		//Return particle effect
		C_ParticleEffect* getParticleEffect() const;
		//Set camera pos
		void setCameraPos(C_Vector3 aC);
		//Draw particles
		//void draw();
		//Set light casters, which calculate to using in shaders
		void setLights(std::vector<C_Light*> aLights);
		void update(const float aTimeTick);
		void draw();
		//Destructor
		~C_ParticleEmitter();
	};

}
