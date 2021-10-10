// Copyright (c) Dan Mitchell
// perfect_shot.cpp
//
// FPS where 'aim and reaction speed don't matter'.
// You equip two weapons. If an enemy is visible in the reticle, constant optimal DPS is done.
// Focus on positioning and movement abilities.

// ------- ------- ------- ------- ------- ------- -------
// headers

#include "common.h"
#include "graphics.h"

// ------- ------- ------- ------- ------- ------- -------
// private declarations

LRESULT CALLBACK window_proc(
	HWND window,
	UINT message,
	WPARAM wparam,
	LPARAM lparam);

// ------- ------- ------- ------- ------- ------- -------
// data

c_d3d_globals g_d3d_globals;

// ------- ------- ------- ------- ------- ------- -------
// private definitions

int CALLBACK wWinMain(
	_In_ HINSTANCE instance,
	_In_opt_ HINSTANCE previous_instance,
	_In_ PWSTR command_line,
	_In_ int command_show)
{
	UNREFERENCED_PARAMETER(previous_instance);
	UNREFERENCED_PARAMETER(command_line);
	UNREFERENCED_PARAMETER(command_show);

	g_d3d_globals.initialize(instance, &window_proc);

	MSG message = {};
	while (message.message != WM_QUIT)
	{
		if (PeekMessage(&message, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&message);
			DispatchMessage(&message);
		}
	}

	g_d3d_globals.destroy();

	return 0;
}

LRESULT CALLBACK window_proc(
	HWND window,
	UINT message,
	WPARAM wparam,
	LPARAM lparam)
{
	bool handled = false;

	if (g_d3d_globals.is_initialized())
	{
		switch (message)
		{
		case WM_PAINT:
			handled = true;
			// Render();
			break;

		case WM_SYSKEYDOWN:
		case WM_KEYDOWN:
		case WM_SYSCHAR:
		case WM_SIZE:
			handled = true;
			break;

		case WM_DESTROY:
			PostQuitMessage(0);
			handled = true;
			break;
		}
	}

	if (handled)
	{
		return 0;
	}
	else
	{
		return DefWindowProc(window, message, wparam, lparam);
	}
}
