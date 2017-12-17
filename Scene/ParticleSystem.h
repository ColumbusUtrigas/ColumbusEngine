#pragma once

#include <Scene/Component.h>
#include <Graphics/Camera.h>
#include <Graphics/Light.h>
#include <Graphics/ParticleEmitter.h>

namespace Columbus
{

	class C_ParticleSystem : public C_Component
	{
	private:
		C_ParticleEmitter* mEmitter = nullptr;
	public:
		C_ParticleSystem(C_ParticleEmitter* aEmitter);

		bool onCreate() override;
		bool onUpdate() override;

		void update(const float aTimeTick) override;
		void render(C_Transform& aTransform) override;
		//This component methods
		std::string getType() override;
		C_ParticleEmitter* getEmitter() const;
		void setLights(std::vector<C_Light*> aLights);
		void setCamera(C_Camera aCamera);
		C_Shader* getShader() const;
		void setShader(C_Shader* aShader);

		~C_ParticleSystem();
	};

}







