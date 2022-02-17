#pragma once

#include <Graphics/Buffer.h>
#include <d3d12.h>

namespace Columbus::Graphics::DX12
{

	class BufferDX12 : public Buffer
	{
	private:
		friend class DeviceDX12;
		friend class DeviceDX12;
		ID3D12Resource* _resource = nullptr;
	public:
		BufferDX12(const BufferDesc& desc)
		{
			this->Desc = desc;
		}

		void* GetHandle() final override
		{
			return _resource;
		}
	};

}
