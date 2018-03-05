#pragma once

#include <Graphics/Device.h>

namespace Columbus
{

	class C_DeviceOpenGL : public C_Device
	{
	public:
		C_DeviceOpenGL();

		C_Cubemap* createCubemap() const override;
		C_Cubemap* createCubemap(std::string aPath) const override;
		C_Cubemap* createCubemap(std::array<std::string, 6> aPath) const override;

		~C_DeviceOpenGL();
	};

}







