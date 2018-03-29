#pragma once

#include <Physics/PhysicsShape.h>
#include <Math/Vector3.h>
#include <Core/Types.h>

namespace Columbus
{

	class PhysicsShapeMultiSphere : public PhysicsShape
	{
	private:
		uint32 Count;
		std::vector<Vector3> Positions;
		std::vector<float> Radiuses;
	public:
		PhysicsShapeMultiSphere(Vector3* Positions, float* Radiuses, uint32 NumSpheres) :
			Count(0)
		{
			if (Positions != nullptr && Radiuses != nullptr && NumSpheres != 0)
			{
				btVector3* bPoses = new btVector3[NumSpheres];
				for (uint32 i = 0; i < NumSpheres; i++)
				{
					bPoses[i].setX(Positions[i].x);
					bPoses[i].setY(Positions[i].y);
					bPoses[i].setZ(Positions[i].z);

					this->Positions.push_back(Positions[i]);
					this->Radiuses.push_back(Radiuses[i]);
				}

				mShape = new btMultiSphereShape(bPoses, Radiuses, (int)NumSpheres);
				Count = NumSpheres;
			}
		}
		/*
		* Returns number of spheres
		* @return uint32: Count of spheres
		*/
		uint32 GetCount() const
		{
			if (mShape != nullptr)
			{
				return Count;
			}

			return 0;
		}
		/*
		* Returns position of sphere
		* @param uint32 Index: Index of the sphere
		* @return Vector3: Position of indexed sphere
		*/
		Vector3 GetPosition(uint32 Index) const
		{
			if (mShape != nullptr && Index < Count)
			{
				return Positions.at(Index);
			}
			
			return Vector3(0, 0, 0);
		}
		/*
		* Returns radius of sphere
		* @param uint32 Index: Index of the sphere
		* @return float: Radius of indexed sphere
		*/
		float GetRadius(uint32 Index) const
		{
			if (mShape != nullptr && Index < Count)
			{
				return Radiuses.at(Index);
			}

			return 0.0f;
		}
	};

}






