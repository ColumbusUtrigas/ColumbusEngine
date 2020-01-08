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
		ComponentBillboard(Billboard&& Bill) : Bill(std::move(Bill)) {}
		virtual void Update(float TimeTick) final override {}

		virtual Type GetType() const final override { return Component::Type::Billboard; }
		Billboard& GetBillboard() { return Bill; }

		~ComponentBillboard() final override {}
	};

}
