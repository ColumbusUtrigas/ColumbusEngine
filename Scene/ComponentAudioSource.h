#pragma once

#include <Scene/Component.h>
#include <Audio/AudioSource.h>

namespace Columbus
{

	class ComponentAudioSource : public Component
	{
	private:
		AudioSource* Source;

		friend class Scene;
	public:
		ComponentAudioSource(AudioSource* InSource) : Source(InSource) {}

		virtual void Update(float TimeTick, Transform& Trans) override
		{
			if (Source != nullptr)
			{
				Source->Position = Trans.GetPos();
			}
		}

		//This component methods
		virtual Type GetType() const override { return Component::Type::AudioSource; }
		AudioSource* GetSource() const { return Source; }

		virtual ~ComponentAudioSource() override {}
	};

}


