#pragma once

#include <Scene/Component.h>
#include <Graphics/Particles/ParticleEmitterCPU.h>

namespace Columbus
{

	class ComponentParticleSystem : public Component
	{
	private:
		ParticleEmitterCPU Emitter;

		friend class Renderer;
		friend class Scene;
	public:
		ComponentParticleSystem(ParticleEmitterCPU&& Particles) : Emitter(std::move(Particles)) {}

		virtual void OnComponentAdd() override
		{
			gameObject->materials.push_back(gameObject->material);
		}

		virtual void Update(float TimeTick, Transform& Trans) override
		{
			Emitter.Position = Trans.Position;
			Emitter.Update(TimeTick);
		}

		ParticleEmitterCPU& GetEmitter()
		{
			return Emitter;
		}

		//This component methods
		virtual Type GetType() const override { return Component::Type::ParticleSystem; }

		virtual ~ComponentParticleSystem() override {}
	};

}


