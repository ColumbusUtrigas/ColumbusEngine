#pragma once

#include <Scene/Component.h>
#include <Graphics/Light.h>
#include <Core/Assert.h>

namespace Columbus
{

	class ComponentLight : public Component
	{
	private:
		Light* LightSource = nullptr;

		friend class Scene;
	public:
		ComponentLight(Light* InLight) : LightSource(InLight)
		{
			COLUMBUS_ASSERT(LightSource != nullptr);
		}

		virtual void Update(float TimeTick, Transform& Trans) override
		{
			COLUMBUS_ASSERT(LightSource != nullptr);
			LightSource->Pos = Trans.Position;
		}

		//This component methods
		virtual Type GetType() const override { return Component::Type::Light; }
		Light* GetLight() const { return LightSource; }

		virtual ~ComponentLight() override
		{
			delete LightSource;
		}
	};

}


