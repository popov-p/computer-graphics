#pragma once
#include "Camera.h"
#include "Input.h"
#include <Windows.h>
#include <dxgi.h>
#include <d3d11.h>
#include <string>
#include <directxmath.h>
#define SAFE_RELEASE(p) if (p) { (p)->Release(); (p) = nullptr; }

struct WorldMatrixBuffer {
	XMMATRIX mWorldMatrix;
};

struct SceneMatrixBuffer {
	XMMATRIX mViewProjectionMatrix;
};

class Renderer {
public:
	Renderer() = default;
	bool Init(HWND hWnd, Camera* pCamera, Input* pInput);
	HRESULT SetupBackBuffer();
	bool Resize(UINT width, UINT height);
	void RenderFrame();

	/*lab-2*/
	HRESULT InitScene();

	/*lab-3*/
	bool GetState();
	~Renderer();
private:
	/*lab-1*/
	ID3D11Device* m_pDevice = NULL;
	ID3D11DeviceContext* m_pDeviceContext = NULL;
	UINT m_width = 0;
	UINT m_height = 0;
	IDXGISwapChain* m_pSwapChain = NULL;
	ID3D11RenderTargetView* m_pBackBufferRTV = NULL;
	/*lab-2*/
	ID3D11Buffer* m_pVertexBuffer = NULL;
	ID3D11Buffer* m_pIndexBuffer = NULL;
	ID3D11VertexShader* m_pVertexShader = nullptr;
	ID3D11PixelShader* m_pPixelShader = nullptr;
	ID3D11InputLayout* m_pInputLayout = nullptr;
	/*lab-3*/
	ID3D11Buffer* m_pWorldMatrixBuffer = nullptr;
	ID3D11Buffer* m_pSceneMatrixBuffer = nullptr;
	ID3D11RasterizerState* m_pRasterizerState = nullptr;

	Camera* m_pCamera = nullptr;
	Input* m_pInput = nullptr;

	void InputMovement();

};