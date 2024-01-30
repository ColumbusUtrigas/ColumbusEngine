#pragma once

#include <Core/Platform.h>

union Float32
{
	unsigned int u;
	float f;
	struct
	{
#if COLUMBUS_LITTLE_ENDIAN
		unsigned int Mantissa : 23;
		unsigned int Exponent : 8;
		unsigned int Sign : 1;
#else
		unsigned int Sign : 1;
		unsigned int Exponent : 8;
		unsigned int Mantissa : 23;
#endif
	} s;
};

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpadded"
#endif

union Float16
{
	unsigned short u;
	struct
	{
#if COLUMBUS_LITTLE_ENDIAN
		unsigned int Mantissa : 10;
		unsigned int Exponent : 5;
		unsigned int Sign : 1;
#else
		unsigned int Sign : 1;
		unsigned int Exponent : 5;
		unsigned int Mantissa : 10;
#endif
	} s;
};

#ifdef __clang__
#pragma clang diagnostic pop
#endif

Float32 Float16to32(Float16 h);
Float16 Float32to16(Float32 f);
