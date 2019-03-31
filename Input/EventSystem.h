#pragma once

#include <functional>
#include <Input/Events.h>

namespace Columbus
{

	class EventSystem
	{
	public:
		std::function<void(const Event&)> QuitFunction;
		std::function<void(const Event&)> InputFunction;
		std::function<void(const Event&)> WindowFunction;

		std::function<void(void*)> RawFunction;
	public:
		void Update();
	};

}


