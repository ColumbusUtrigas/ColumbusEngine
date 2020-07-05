#include <Core/Util.h>

namespace Columbus
{

	const char* HumanizeBytes(uint64 bytes, double& dsize)
	{
		const char* const sizes[] = { "bytes", "KB", "MB", "GB", "TB", "PB" };
		int sindex = 0;
		dsize = bytes;
		while (bytes >= 1024)
		{
			dsize /= 1024.0;
			bytes = dsize + 1;
			sindex++;
		}

		return sizes[sindex];
	}

}
