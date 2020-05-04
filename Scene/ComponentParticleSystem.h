#pragma once

#include <Scene/Component.h>
#include <Graphics/Particles/ParticleEmitterCPU.h>
#include <Scene/GameObject.h>

namespace Columbus
{

	class ComponentParticleSystem : public Component
	{
		DECLARE_COMPONENT(ComponentParticleSystem, ParticleEmitterCPU{});
	private:
		ParticleEmitterCPU Emitter;

		friend class Renderer;
		friend class Scene;
	public:
		ComponentParticleSystem(ParticleEmitterCPU&& Particles) : Emitter(std::move(Particles)) {}

		Component* Clone() const final override
		{
			return new ComponentParticleSystem({});
		}

		virtual void OnComponentAdd() override
		{
			gameObject->materials.push_back(gameObject->material);
		}

		virtual void Update(float TimeTick) override
		{
			Emitter.Position = gameObject->transform.Position;
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


