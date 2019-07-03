#pragma once

#include <Graphics/Light.h>
#include <Common/Model/Model.h>
#include <Math/Box.h>

#include <vector>

namespace Columbus
{

	class Mesh
	{
	protected:
		Box BoundingBox;
	public:
		Mesh() {}

		bool Load(const char* FileName)
		{
			Model TmpModel;

			if (!TmpModel.Load(FileName))
			{
				return false;
			}

			return Load(TmpModel);
		}

		virtual bool Load(const Model& InModel) = 0;
		virtual void Bind() = 0;
		virtual uint32 Render() = 0;
		virtual void Unbind() = 0;

		Box GetBoundingBox() const { return BoundingBox; }

		void Clear() {}

		virtual ~Mesh() {};
	};

}


