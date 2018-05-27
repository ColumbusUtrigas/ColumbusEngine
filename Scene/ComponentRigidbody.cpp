#include <Scene/ComponentRigidbody.h>

namespace Columbus
{

	ComponentRigidbody::ComponentRigidbody(Rigidbody* RB) : RB(RB) {}
	bool ComponentRigidbody::OnCreate() { return false; }
	bool ComponentRigidbody::OnUpdate() { return false; }
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





