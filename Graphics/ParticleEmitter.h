#include <cstdio>
#include <cstdlib>
#include <vector>

#include <System/System.h>
#include <Math/Vector2.h>
#include <Math/Vector3.h>
#include <Math/Vector4.h>
#include <Graphics/Shader.h>
#include <Graphics/Buffer.h>
#include <Graphics/Camera.h>

namespace C
{

	typedef struct
	{
		C_Vector3 pos = C_Vector3(0, 2, 0);
		C_Vector3 vel = C_Vector3(0, 0, 0);
		C_Vector3 speed = C_Vector3(0, 0.03, 0);
		
		float lifetime = 60;
		float age = 0;
		
		inline void reset()
		{
			pos = C_Vector3(0, 2, 0);
			vel = C_Vector3(0, 0, 0);
			speed = C_Vector3(0, 0.03, 0);
			
			age = 0;
			lifetime = 60;
		}
	} C_Particle;
	
	class C_ParticleEmitter
	{
	private:
		std::vector<C_Particle> mParticles;
		
		size_t mSize = 100;
		
		C_Shader* mShader = NULL;
		
		C_Buffer* mBuf = NULL;
		C_Buffer* mTBuf = NULL;
	public:
		C_ParticleEmitter(size_t aSize = 1);
		
		void draw();
		
		~C_ParticleEmitter();
	};

}







