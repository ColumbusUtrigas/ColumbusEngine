#pragma once

#include <System/File.h>
#include <stb_image.h>

namespace Columbus
{
	static stbi_io_callbacks StreamToStbCallbacks(DataStream& Stream)
	{
		stbi_io_callbacks Callbacks;
		Callbacks.skip = [](void* user, int n) { reinterpret_cast<DataStream*>(user)->SeekCur(n); };
		Callbacks.eof = [](void* user) -> int { return reinterpret_cast<DataStream*>(user)->IsEOF() ? 1 : 0; };
		Callbacks.read = [](void* user, char* data, int size) { return (int)reinterpret_cast<DataStream*>(user)->Read(data, 1, size); };

		return Callbacks;
	}
}
