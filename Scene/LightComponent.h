#pragma once

#include <Scene/Component.h>
#include <Graphics/Light.h>

namespace Columbus
{

	class LightComponent : public Component
	{
	private:
		C_Light* mLight = nullptr;
	public:
		LightComponent(C_Light* aLight);

		bool onCreate() override;
		bool onUpdate() override;

		void update(const float aTimeTick) override;
		void render(Transform& aTransform) override;
		//This component methods
		std::string getType() override;
		C_Light* getLight() const;

		~LightComponent();
	};

}



