#pragma once

#include <Core/Function.h>
#include <list>

template <typename...Args>
struct Event
{

private:
	std::list<Fun<Args...>> _internal;
};
