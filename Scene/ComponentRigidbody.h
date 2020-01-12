#pragma once

#include <Scene/Component.h>
#include <Physics/Rigidbody.h>

namespace Columbus
{

	class ComponentRigidbody : public Component
	{
	private:
		Rigidbody* RB;
	public:
		ComponentRigidbody(Rigidbody* RB) : RB(RB) {}

		Component* Clone() const final override
		{
			return new ComponentRigidbody(RB);
		}

		virtual void Update(float TimeTick) override {}

		//This component methods
		virtual Component::Type GetType() const override { return Component::Type::Rigidbody; }
		Rigidbody* GetRigidbody() const { return RB; }

		virtual ~ComponentRigidbody() override {}
	};

}


