#pragma once

#include <btBulletDynamicsCommon.h>
#include <Core/PrototypeFactory.h>
#include <Core/ISerializable.h>
#include <Core/ICloneable.h>

namespace Columbus
{

	class PhysicsShape : public ISerializable, public ICloneable
	{
	protected:
		float Margin;

		void BaseSerialize(JSON& J) const;
		void BaseDeserialize(JSON& J);
	public:
		btCollisionShape* mShape;
	public:
		PhysicsShape() : Margin(0.0f), mShape(nullptr) {}
		/*
		* Returns true if Shape exits, else returns false
		* @return bool: Exits-value
		*/
		virtual bool IsExist() const
		{
			return mShape != nullptr;
		}
		/*
		* Sets collision margin into collision shape
		* @param float Margin: New collision margin of shape
		*/
		virtual void SetMargin(float Margin)
		{
			if (mShape != nullptr)
			{
				mShape->setMargin(Margin);
				this->Margin = Margin;
			}
		}
		/*
		* Returns collision margin of collision shape
		* @return float: Collision margin of shape
		*/
		virtual float GetMargin() const
		{
			if (mShape != nullptr)
			{
				return Margin;
			}

			return 0.0f;
		}

		virtual std::string_view GetTypename() const = 0;

		virtual void Serialize(JSON& J) const override = 0;
		virtual void Deserialize(JSON& J) override = 0;

		virtual ~PhysicsShape() { delete mShape; }
	};
}
