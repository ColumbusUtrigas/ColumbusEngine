#pragma once

namespace Columbus
{

	struct InputLayoutElementDesc
	{
		// semantic name
		// semantic index
		// format
		uint32 Slot;
		// offset
		// class
		// instance data step
		uint32 Components; //float!!!
	};

	struct InputLayout
	{
		InputLayoutElementDesc Elements[16];
		uint32 NumElements = 0;
	};

}
