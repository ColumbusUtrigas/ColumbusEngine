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
#include <Graphics/Particles/ParticleEffect.h>
#include <Graphics/Particles/Particle.h>
#include <Graphics/Light.h>
#include <System/System.h>
#include <System/Timer.h>
#include <System/Random.h>
#include <Common/Noise/OctaveNoise.h>

namespace Columbus
{

	struct ColorKey
	{
		Vector4 color = Vector4(1, 1, 1, 1);
		float key = 0.0;
	};

	class ParticleEmitter
	{
	private:
		ParticleEffect* mParticleEffect = nullptr;

		std::vector<Particle> mParticles;
		std::vector<Particle> mActiveParticles;
		std::vector<Light*> mLights;
		std::vector<ColorKey> mColorKeys;

		C_Buffer* mBuf = nullptr;
		C_Buffer* mTBuf = nullptr;
		C_Buffer* mCBuf = nullptr;
		C_Buffer* mPBuf = nullptr;
		C_Buffer* mLBuf = nullptr;
		C_Buffer* mSBuf = nullptr;

		OctaveNoise mNoise;

		float mLife = 0.0;
		
		Camera mCamera;

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
		void setBuffers();
		void setUniforms();
		void setShaderMaterial();
		void setShaderLightAndCamera();
		void calculateLights();
		void sortLights();
		void unbindAll();
		
		float mLightUniform[120];
		float mTimer = 0.0;
		float* mVertData = nullptr;
		float* mUvData = nullptr;
		float* mColData = nullptr;
		float* mPosData = nullptr;
		float* mTimeData = nullptr;
		float* mSizeData = nullptr;
		unsigned int mParticlesCount = 0;
	public:
		ParticleEmitter(const ParticleEffect* aParticleEffect);

		void setParticleEffect(const ParticleEffect* aParticleEffect);
		ParticleEffect* getParticleEffect() const;

		void update(const float aTimeTick);
		void draw();

		void setCamera(const Camera aCamera);
		//Set light casters, which calculate to using in shaders
		void setLights(std::vector<Light*> aLights);

		~ParticleEmitter();
	};

}
