#include "input.h"

Input::Input(HINSTANCE hinst, HWND hwnd, int screenW, int screenH) : m_hinst(hinst), m_hwnd(hwnd), m_screenW(screenW), m_screenH(screenH) {
	m_state = true;
	m_mouseState = XMFLOAT3(0.0f, 0.0f, 0.0f);
}

Input::~Input() {

};

bool Input::readKB() {
	return true;
}

bool Input::readMouse() {
	POINT pt;
	GetCursorPos(&pt);
	ScreenToClient(m_hwnd, &pt);

	prevMouseState = m_mouseState;
	m_mouseState.x = static_cast<float>(pt.x);
	m_mouseState.y = static_cast<float>(pt.y);

	return true;
}

bool Input::getState() {
	return readKB() && readMouse();
}

XMFLOAT3 Input::getMouseState() {
	XMFLOAT3 mouseDelta = XMFLOAT3(
		m_mouseState.x - prevMouseState.x,
		m_mouseState.y - prevMouseState.y,
		m_mouseState.z - prevMouseState.z);

	BYTE keyState[256];
	auto st = GetKeyboardState(keyState);
	if (!st) {
		return XMFLOAT3(0.0f, 0.0f, 0.0f);
	}
	if (keyState[VK_UP] & 0x80) {
		mouseDelta.z = 1.0f;
		return mouseDelta;
	}

	if (keyState[VK_DOWN] & 0x80) {
		mouseDelta.z = -1.0f;
		return mouseDelta;
	}

	if (keyState[VK_LBUTTON] & 0x80)
		return mouseDelta;
	return XMFLOAT3(0.0f, 0.0f, 0.0f);
}

void Input::resize(int screenW, int screenH) {
	m_screenH = screenH;
	m_screenW = screenW;
}