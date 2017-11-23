#pragma once

#include <Scene/Component.h>
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

		~C_ParticleSystem();
	};

}







