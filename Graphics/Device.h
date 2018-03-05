#pragma once

#include <Graphics/Cubemap.h>

namespace Columbus
{

	class C_Device
	{
	public:
		C_Device();

		virtual C_Cubemap* createCubemap() const;
		virtual C_Cubemap* createCubemap(std::string aPath) const;
		virtual C_Cubemap* createCubemap(std::array<std::string, 6> aPath) const;

		~C_Device();
	};

	extern C_Device* gDevice;

}










