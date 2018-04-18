#include <Scene/ComponentLight.h>

namespace Columbus
{

	//////////////////////////////////////////////////////////////////////////////
	ComponentLight::ComponentLight(Light* aLight) :
		mLight(aLight)
	{

	}
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	bool ComponentLight::OnCreate()
	{
		return false;
	}
	//////////////////////////////////////////////////////////////////////////////
	bool ComponentLight::OnUpdate()
	{
		return false;
	}
	//////////////////////////////////////////////////////////////////////////////
	void ComponentLight::Update(const float aTimeTick)
	{
	
	}
	//////////////////////////////////////////////////////////////////////////////
	void ComponentLight::Render(Transform& aTransform)
	{
		if (mLight != nullptr)
		{
			mLight->setPos(aTransform.GetPos());
		}
	}
	//////////////////////////////////////////////////////////////////////////////
	Component::Type ComponentLight::GetType() const
	{
		return COMPONENT_LIGHT;
	}
	//////////////////////////////////////////////////////////////////////////////
	Light* ComponentLight::GetLight() const
	{
		return mLight;
	}
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	ComponentLight::~ComponentLight()
	{

	}

}



