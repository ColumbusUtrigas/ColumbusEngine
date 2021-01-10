#pragma once

#include <Core/DragDrop.h>
#include <ole2.h>

struct WindowsDragDropManager : public IDragDropManager, IDropTarget
{
	~WindowsDragDropManager() override {}
public:
	//--- inherited from IDropTarget (Win32 OLE2)
	// we handle drop targets, let others kno
	HRESULT QueryInterface(REFIID riid, void** ppvObject);
	// occurs when we drag files into our applications view
	HRESULT DragEnter(IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect);
	// occurs when we drag files out from our applications view
	HRESULT DragLeave();
	// occurs when we drag the mouse over our applications view whilst carrying files (post Enter, pre Leave)
	HRESULT DragOver(DWORD grfKeyState, POINTL pt, DWORD* pdwEffect);
	// occurs when we release the mouse button to finish the drag-drop operation
	HRESULT Drop(IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect);

	//--- implement the IUnknown parts
	// you could do this the proper way with InterlockedIncrement etc,
	// but I've left out stuff that's not exactly necessary for brevity
	ULONG AddRef() { return 1; }
	ULONG Release() { return 0; }
};
