#pragma once

namespace Columbus
{

	class IComputePipelineState
	{
	public:
		virtual void* GetHandle() = 0;
	};

}
