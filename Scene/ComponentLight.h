#pragma once

#include <Scene/Component.h>
#include <Graphics/Light.h>

namespace Columbus
{

	class ComponentLight : public Component
	{
	private:
		Light* mLight = nullptr;
	public:
		ComponentLight(Light* aLight);

		bool OnCreate() override;
		bool OnUpdate() override;

		void Update(const float aTimeTick) override;
		void Render(Transform& aTransform) override;
		//This component methods
		Type GetType() const override;
		Light* GetLight() const;

		~ComponentLight() override;
	};

}



