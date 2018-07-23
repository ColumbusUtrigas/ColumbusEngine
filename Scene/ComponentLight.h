#pragma once

#include <Scene/Component.h>
#include <Graphics/Light.h>

namespace Columbus
{

	class ComponentLight : public Component
	{
	private:
		Light* LightSource = nullptr;
	public:
		ComponentLight(Light* InLight);

		void Update(const float TimeTick) override;
		void Render(Transform& Trans) override;
		//This component methods
		Type GetType() const override;
		Light* GetLight() const;

		~ComponentLight() override;
	};

}



