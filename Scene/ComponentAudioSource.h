#pragma once

#include <Scene/Component.h>
#include <Audio/AudioSource.h>

namespace Columbus
{

	class ComponentAudioSource : public Component
	{
	private:
		AudioSource* Source;
	public:
		ComponentAudioSource(AudioSource* InSource) : Source(InSource) {}

		virtual void Render(Transform& Trans) override {}
		virtual void Update(float TimeTick, Transform& Trans) override
		{
			if (Source != nullptr)
			{
				Source->SetPosition(Trans.GetPos());
			}
		}

		//This component methods
		virtual Type GetType() const override { return Component::Type::AudioSource; }
		AudioSource* GetSource() const { return Source; }

		virtual ~ComponentAudioSource() override {}
	};

}












