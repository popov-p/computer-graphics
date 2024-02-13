#pragma once

#include <Windows.h>
#include <dxgi.h>
#include <d3d11.h>

#define SAFE_RELEASE(p) if (p) { (p)->Release(); (p) = nullptr; }

class Renderer {
public:
	Renderer() = default;
	bool Init(HWND hWnd);
	HRESULT SetupBackBuffer();
	bool Resize(UINT width, UINT height);
	void RenderFrame();
	void ReleasePointers();
private:
	ID3D11Device* m_pDevice = NULL;
	ID3D11DeviceContext* m_pDeviceContext = NULL;
	UINT m_width = 16;
	UINT m_height = 16;
	IDXGISwapChain* m_pSwapChain = NULL;
	ID3D11RenderTargetView* m_pBackBufferRTV = NULL;
};