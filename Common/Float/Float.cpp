#include "Float.h"

Float32 Float16to32(Float16 h)
{
	static const Float32 magic = { 113 << 23 };
	static const unsigned int shifted_exp = 0x7c00
		<< 13;  // exponent mask after shift
	Float32 o;

	o.u = (h.u & 0x7fffU) << 13U;           // exponent/mantissa bits
	unsigned int exp_ = shifted_exp & o.u;  // just the exponent
	o.u += (127 - 15) << 23;                // exponent adjust

	// handle exponent special cases
	if (exp_ == shifted_exp)    // Inf/NaN?
		o.u += (128 - 16) << 23;  // extra exp adjust
	else if (exp_ == 0)         // Zero/Denormal?
	{
		o.u += 1 << 23;  // extra exp adjust
		o.f -= magic.f;  // renormalize
	}

	o.u |= (h.u & 0x8000U) << 16U;  // sign bit
	return o;
}

Float16 Float32to16(Float32 f)
{
    Float16 o = { 0 };

    // Based on ISPC reference code (with minor modifications)
    if (f.s.Exponent == 0)  // Signed zero/denormal (which will underflow)
        o.s.Exponent = 0;
    else if (f.s.Exponent == 255)  // Inf or NaN (all exponent bits set)
    {
        o.s.Exponent = 31;
        o.s.Mantissa = f.s.Mantissa ? 0x200 : 0;  // NaN->qNaN and Inf->Inf
    }
    else                                      // Normalized number
    {
        // Exponent unbias the single, then bias the halfp
        int newexp = f.s.Exponent - 127 + 15;
        if (newexp >= 31)  // Overflow, return signed infinity
            o.s.Exponent = 31;
        else if (newexp <= 0)  // Underflow
        {
            if ((14 - newexp) <= 24)  // Mantissa might be non-zero
            {
                unsigned int mant = f.s.Mantissa | 0x800000;  // Hidden 1 bit
                o.s.Mantissa = mant >> (14 - newexp);
                if ((mant >> (13 - newexp)) & 1)  // Check for rounding
                    o.u++;  // Round, might overflow into exp bit, but this is OK
            }
        }
        else
        {
            o.s.Exponent = static_cast<unsigned int>(newexp);
            o.s.Mantissa = f.s.Mantissa >> 13;
            if (f.s.Mantissa & 0x1000)  // Check for rounding
                o.u++;                    // Round, might overflow to inf, this is OK
        }
    }

    o.s.Sign = f.s.Sign;
    return o;
}