#include <Scene/ComponentLight.h>

namespace Columbus
{

	ComponentLight::ComponentLight(Light* InLight) :
		LightSource(InLight)
	{

	}

	void ComponentLight::Update(const float TimeTick)
	{
		
	}

	void ComponentLight::Render(Transform& Trans)
	{
		if (LightSource != nullptr)
		{
			LightSource->setPos(Trans.GetPos());
		}
	}
	
	Component::Type ComponentLight::GetType() const
	{
		return Component::Type::Light;
	}
	
	Light* ComponentLight::GetLight() const
	{
		return LightSource;
	}
	
	ComponentLight::~ComponentLight()
	{

	}

}




