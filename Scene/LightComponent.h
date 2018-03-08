#pragma once

#include <Scene/Component.h>
#include <Graphics/Light.h>

namespace Columbus
{

	class LightComponent : public Component
	{
	private:
		Light* mLight = nullptr;
	public:
		LightComponent(Light* aLight);

		bool onCreate() override;
		bool onUpdate() override;

		void update(const float aTimeTick) override;
		void render(Transform& aTransform) override;
		//This component methods
		std::string getType() override;
		Light* getLight() const;

		~LightComponent();
	};

}



