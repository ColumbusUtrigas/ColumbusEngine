#pragma once

#include <Scene/Component.h>
#include <Physics/Rigidbody.h>
#include <functional>

namespace Columbus
{

	class ComponentRigidbody : public Component
	{
		DECLARE_COMPONENT(ComponentRigidbody, new Rigidbody());
	private:
		Rigidbody* RB;

		friend class Scene;
		static std::function<void(Rigidbody* RB)> OnAdd;
	public:
		ComponentRigidbody(Rigidbody* RB) : RB(RB) {}

		Component* Clone() const final override
		{
			return new ComponentRigidbody(new Rigidbody());
		}

		virtual void Update(float TimeTick) override {}
		virtual void OnComponentAdd() override
		{
			OnAdd(RB);
		}

		//This component methods
		virtual Component::Type GetType() const override { return Component::Type::Rigidbody; }
		Rigidbody* GetRigidbody() const { return RB; }

		virtual ~ComponentRigidbody() override {}
	};

}


