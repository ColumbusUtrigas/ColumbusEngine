#pragma once

#include <Core/Types.h>
#include <Core/fixed_vector.h>

namespace Columbus
{

	struct InputLayoutElementDesc
	{
		const char* SemanticName;
		uint32 SemanticIndex;
		// format
		uint32 Slot;
		// offset
		// class
		// instance data step
		uint32 Components; //float!!!

		InputLayoutElementDesc() {}
		InputLayoutElementDesc(const char* semanticName, uint32 semanticIndex, uint32 slot, uint32 components) :
			SemanticName(semanticName), SemanticIndex(semanticIndex), Slot(slot), Components(components) {}
	};

	struct InputLayout
	{
		fixed_vector<InputLayoutElementDesc, 16> Elements;
	};

}
