#pragma once

#include <Scene/Component.h>
#include <Scene/GameObject.h>
#include <Audio/AudioSource.h>
#include <Core/Assert.h>
#include <memory>

namespace Columbus
{

	class ComponentAudioSource : public Component
	{
		DECLARE_COMPONENT(ComponentAudioSource, std::make_shared<AudioSource>());
	private:
		std::shared_ptr<AudioSource> Source;

		friend class Scene;
	public:
		ComponentAudioSource(std::shared_ptr<AudioSource> InSource) : Source(InSource)
		{
			COLUMBUS_ASSERT((bool)Source);
		}

		virtual void Update(float TimeTick) override
		{
			COLUMBUS_ASSERT((bool)Source);
			Source->Position = gameObject->transform.Position;
		}

		Component* Clone() const final override
		{
			return new ComponentAudioSource(Source);
		}

		//This component methods
		virtual Type GetType() const override { return Component::Type::AudioSource; }
		auto GetSource() const { return Source; }

		virtual ~ComponentAudioSource() override {}
	};

}


