#pragma once

#include <Scene/Component.h>
#include <Graphics/Billboard.h>

namespace Columbus
{

	class ComponentBillboard : public Component
	{
	private:
		Billboard Bill;
	public:
		virtual void Update(float TimeTick) final override {}

		Component* Clone() const final override
		{
			auto n = new ComponentBillboard();
			n->Bill = Bill;
			return n;
		}

		virtual Type GetType() const final override { return Component::Type::Billboard; }
		Billboard& GetBillboard() { return Bill; }

		~ComponentBillboard() final override {}
	};

}
