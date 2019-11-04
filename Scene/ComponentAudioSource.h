#pragma once

#include <Scene/Component.h>
#include <Audio/AudioSource.h>
#include <Core/Assert.h>

namespace Columbus
{

	class ComponentAudioSource : public Component
	{
	private:
		AudioSource* Source;

		friend class Scene;
	public:
		ComponentAudioSource(AudioSource* InSource) : Source(InSource)
		{
			COLUMBUS_ASSERT(Source != nullptr);
		}

		virtual void Update(float TimeTick, Transform& Trans) override
		{
			COLUMBUS_ASSERT(Source != nullptr);
			Source->Position = Trans.Position;
		}

		//This component methods
		virtual Type GetType() const override { return Component::Type::AudioSource; }
		AudioSource* GetSource() const { return Source; }

		virtual ~ComponentAudioSource() override {}
	};

}


