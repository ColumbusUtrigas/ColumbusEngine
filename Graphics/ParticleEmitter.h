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
		
		inline ~C_Particle() {}
	};
	
	class C_ParticleEmitter
	{
	private:
		std::vector<C_Particle> mParticles;
		
		size_t mSize = 100;
		
		C_Shader* mShader = NULL;
		
		C_Buffer* mBuf = NULL;
		C_Buffer* mTBuf = NULL;
		
		C_Timer tm;
		
		C_Vector3 mPos = C_Vector3(0, 0, 0);
		float mAcc = 0.0;
		float mSpeed = 1.0;
		float mLifetime = 1.0;
	public:
		C_ParticleEmitter(size_t aSize = 100, C_Vector3 aPos = C_Vector3(0, 0, 0), float aAcc = 0.0, float aSpd = 1.0, float aTTL = 1.0);
		
		void draw();
		
		~C_ParticleEmitter();
	};

}







