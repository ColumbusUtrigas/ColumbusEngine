#pragma once

#include <Scene/Component.h>
#include <Scene/GameObject.h>
#include <Graphics/Light.h>

namespace Columbus
{

	class ComponentLight : public Component
	{
		DECLARE_COMPONENT(ComponentLight);
	private:
		Light LightSource;
	public:
		virtual void Update(float TimeTick) final override
		{
			LightSource.Pos = gameObject->transform.Position;
		}

		Component* Clone() const final override
		{
			auto n = new ComponentLight();
			n->LightSource = LightSource;
			return n;
		}

		//This component methods
		virtual Type GetType() const override { return Component::Type::Light; }
		Light& GetLight() { return LightSource; }

		virtual ~ComponentLight() override {}
	};

}
