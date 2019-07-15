#pragma once

namespace Columbus
{

	class JSON;

	struct ISerializable
	{
		virtual void Serialize(JSON& J) const = 0;
		virtual void Deserialize(JSON& J) = 0;
	};

}
