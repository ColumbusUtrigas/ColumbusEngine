#pragma once

#include <Scene/Component.h>
#include <Graphics/Light.h>

namespace Columbus
{

	class C_LightComponent : public C_Component
	{
	private:
		C_Light* mLight = nullptr;
	public:
		C_LightComponent(C_Light* aLight);

		bool onCreate() override;
		bool onUpdate() override;

		void update(const float aTimeTick) override;
		void render(C_Transform& aTransform) override;
		//This component methods
		std::string getType() override;
		C_Light* getLight() const;

		~C_LightComponent();
	};

}



