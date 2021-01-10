#pragma once

#include <Core/SmartPointer.h>
#include <Core/Function.h>

struct IDragDropManager
{
	Fun<void()> onEnter;
	Fun<void()> onDrag;
	Fun<void()> onLeave;
	Fun<void()> onDrop;

	virtual ~IDragDropManager() {}
};
