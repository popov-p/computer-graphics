#pragma once

#include <dinput.h>
#include <directxmath.h>
#include <windows.h>

using namespace DirectX;

class Input {
private:
	bool readKB();
	bool readMouse();
	HINSTANCE m_hinst;
	HWND m_hwnd;
	int m_screenW;
	int m_screenH;
	bool m_state;
	XMFLOAT3 m_mouseState;
	XMFLOAT3 prevMouseState = XMFLOAT3(0.0f, 0.0f, 0.0f);

public:
	Input(HINSTANCE hinst, HWND hwnd, int screenW, int screenH);
	~Input();
	bool getState();

	XMFLOAT3 getMouseState();

	void resize(int screenW, int screenH);
};