#pragma once

#include <Graphics/Core/Buffer.h>
#include <Math/Matrix.h>
#include <vector>

namespace Columbus
{

	enum class AccelerationStructureType
	{
		BLAS, // Bottom-level
		TLAS, // Top-level
	};

	class AccelerationStructure;

	struct AccelerationStructureInstance
	{
		Matrix Transform;
		AccelerationStructure* Blas;
	};

	struct AccelerationStructureDesc
	{
		AccelerationStructureType Type;

		// BLAS-only
		Buffer* Vertices;
		Buffer* Indices;
		int VerticesCount;
		int IndicesCount;

		// TLAS-only
		std::vector<AccelerationStructureInstance> Instances;
	};

	class AccelerationStructure
	{
	protected:
		AccelerationStructureDesc _Desc;
		AccelerationStructure(const AccelerationStructureDesc& Desc) : _Desc(Desc) {}

		u64 Size = 0;
	public:
		const AccelerationStructureDesc& GetDesc() const { return _Desc; }

		u64 GetSize() const { return Size; }
	};

}
