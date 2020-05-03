#pragma once

namespace Columbus
{

	class ICloneable
	{
	public:
		virtual ICloneable* Clone() const = 0;
	};

}
