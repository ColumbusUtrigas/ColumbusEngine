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
		ComponentRigidbody(Rigidbody* RB);

		void Update(const float TimeTick) override;
		void Render(Transform& Transform) override;
		//This component methods
		Component::Type GetType() const override;
		Rigidbody* GetRigidbody() const;

		~ComponentRigidbody() override;
	};

}






