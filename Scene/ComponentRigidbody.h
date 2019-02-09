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

		virtual void Update(float TimeTick, Transform& Trans) override {}
		virtual void Render(Transform& Transform) override {}

		//This component methods
		virtual Component::Type GetType() const override { return Component::Type::Rigidbody; }
		Rigidbody* GetRigidbody() const { return RB; }

		virtual ~ComponentRigidbody() override {}
	};

}






