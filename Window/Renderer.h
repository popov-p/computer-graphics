#pragma once

#include <Windows.h>
#include <dxgi.h>
#include <d3d11.h>
#include <string>
#include <directxmath.h>
#define SAFE_RELEASE(p) if (p) { (p)->Release(); (p) = nullptr; }

class Renderer {
public:
	Renderer() = default;
	bool Init(HWND hWnd);
	HRESULT SetupBackBuffer();
	bool Resize(UINT width, UINT height);
	void RenderFrame();
	void ReleasePointers();

	/*lab-2*/
	HRESULT InitScene();

private:
	/*lab-1*/
	ID3D11Device* m_pDevice = NULL;
	ID3D11DeviceContext* m_pDeviceContext = NULL;
	UINT m_width = 16;
	UINT m_height = 16;
	IDXGISwapChain* m_pSwapChain = NULL;
	ID3D11RenderTargetView* m_pBackBufferRTV = NULL;
	/*lab-2*/
	ID3D11Buffer* m_pVertexBuffer = NULL;
	ID3D11Buffer* m_pIndexBuffer = NULL;
	ID3D11VertexShader* m_pVertexShader = nullptr;
	ID3D11PixelShader* m_pPixelShader = nullptr;
};