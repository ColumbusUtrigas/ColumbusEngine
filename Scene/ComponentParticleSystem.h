#pragma once

#include <Scene/Component.h>
#include <Graphics/Camera.h>
#include <Graphics/Light.h>
#include <Graphics/Particles/ParticleEmitter.h>

namespace Columbus
{

	class ComponentParticleSystem : public Component
	{
	private:
		ParticleEmitter* Emitter = nullptr;
	public:
		ComponentParticleSystem(ParticleEmitter* Emitter);

		bool OnCreate() override;
		bool OnUpdate() override;

		void Update(const float TimeTick) override;
		void Render(Transform& Transform) override;
		//This component methods
		Type GetType() const override;
		ParticleEmitter* GetEmitter() const;
		void SetLights(std::vector<Light*> Lights);
		void SetCamera(Camera Camera);
		ShaderProgram* GetShader() const;
		void SetShader(ShaderProgram* Shader);

		~ComponentParticleSystem() override;
	};

}







