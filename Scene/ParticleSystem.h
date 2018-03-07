#pragma once

#include <Scene/Component.h>
#include <Graphics/Camera.h>
#include <Graphics/Light.h>
#include <Graphics/ParticleEmitter.h>

namespace Columbus
{

	class ParticleSystem : public Component
	{
	private:
		C_ParticleEmitter* mEmitter = nullptr;
	public:
		ParticleSystem(C_ParticleEmitter* aEmitter);

		bool onCreate() override;
		bool onUpdate() override;

		void update(const float aTimeTick) override;
		void render(Transform& aTransform) override;
		//This component methods
		std::string getType() override;
		C_ParticleEmitter* getEmitter() const;
		void setLights(std::vector<C_Light*> aLights);
		void setCamera(C_Camera aCamera);
		Shader* getShader() const;
		void setShader(Shader* aShader);

		~ParticleSystem();
	};

}







