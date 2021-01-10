#ifndef _MAIN_H_
#define _MAIN_H_

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN    // Exclude rarely-used stuff from Windows headers.
#endif

#include <Core/Core.h>

#define DX12_ENABLE_DEBUG_LAYER 1

#if COLUMBUS_EDITOR
	#include <Editor/Settings.h>
	#include "EditorMain.h"
#endif

#include <Graphics/Types.h>
#include <Core/Assert.h>
#include <windows.h>

extern LPCTSTR window_class;
extern LPCTSTR window_title;
extern HWND hwnd;
extern HINSTANCE hinstance;
extern Columbus::GraphicsAPI chosen_api;

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// API-independent
bool InitializeEngine();
void NewFrameImGUI();
void BeginFrame();
void Render();
void ShutdownEngine();

// API-dependent
void InitializeCommon();

#ifdef COLUMBUS_EDITOR
	void InitializeImGUI_Common();
	void InitializeImGUI_DX12();
	void InitializeImGUI_GL();

	void NewFrameImGUI_DX12();
	void NewFrameImGUI_GL();

	void RenderImGUI_DX12();
	void RenderImGUI_GL();

	void ShutdownImGUI_DX12();
	void ShutdownImGUI_GL();
#endif

bool InititalizeWindowAndContext_DX12();
bool InitializeWindowAndContext_GL();

void ResizeSwapchain_DX12(HWND hwnd, WPARAM wparam, LPARAM lparam);

void BeginFrame_DX12();
void BeginFrame_GL();

void Present_DX12();
void Present_GL();

void ShutdownWindowAndContext_DX12();
void ShutdownWindowAndContext_GL();

#endif
