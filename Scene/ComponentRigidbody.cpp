#include <Scene/ComponentRigidbody.h>

namespace Columbus
{

	ComponentRigidbody::ComponentRigidbody(Rigidbody* RB) : RB(RB) {}
	void ComponentRigidbody::Update(const float TimeTick) {}

	void ComponentRigidbody::Render(Transform& Transform)
	{
		//FUCK THIS SHIT
		//RB->SetTransform(Transform);
	}
	/*
	* This component methods
	*/
	Component::Type ComponentRigidbody::GetType() const
	{
		return Component::Type::Rigidbody;
	}

	Rigidbody* ComponentRigidbody::GetRigidbody() const
	{
		return RB;
	}

	ComponentRigidbody::~ComponentRigidbody() {}

}





