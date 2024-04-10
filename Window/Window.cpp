#include "renderer.h"

HINSTANCE g_hInst = nullptr;
UINT WindowWidth = 1280;
UINT WindowHeight = 720;

HRESULT InitWindow(HINSTANCE hInstance, int nCmdShow);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

Renderer* pRenderer = nullptr;

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	if (FAILED(InitWindow(hInstance, nCmdShow)))
		return 0;

	MSG msg = { 0 };
	while (WM_QUIT != msg.message)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		if (pRenderer->GetState())
			pRenderer->RenderFrame();
	}
	delete pRenderer;

	return (int)msg.wParam;
}

HRESULT InitWindow(HINSTANCE hInstance, int nCmdShow) {
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = 0;
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = nullptr;
	wcex.lpszClassName = L"WindowClass";
	wcex.hIconSm = 0;

	if (!RegisterClassEx(&wcex))
		return E_FAIL;

	g_hInst = hInstance;
	HWND hWnd = CreateWindow(L"WindowClass", L"Window",
		WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr,
		nullptr, hInstance, nullptr);
	if (!hWnd) {
		return E_FAIL;
	}

	ShowWindow(hWnd, nCmdShow);
	SetForegroundWindow(hWnd);
	SetFocus(hWnd);
	UpdateWindow(hWnd);

	{
		RECT rc;
		rc.left = 0;
		rc.right = WindowWidth;
		rc.top = 0;
		rc.bottom = WindowHeight;

		AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, TRUE);

		MoveWindow(hWnd, 100, 100, rc.right - rc.left, rc.bottom - rc.top, TRUE);

	}

	pRenderer = new Renderer();
	if (!pRenderer->Init(hWnd, new Camera, new Input(hInstance, hWnd, WindowWidth, WindowHeight))) {
		delete pRenderer;
		return E_FAIL;
	}

	return S_OK;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	case WM_SIZE:
		if (pRenderer != nullptr) {
			RECT rc;
			GetClientRect(hWnd, &rc);
			pRenderer->Resize(rc.right - rc.left, rc.bottom - rc.top);
		}
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}