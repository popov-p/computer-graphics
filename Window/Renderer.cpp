#include "Renderer.h"
#include "framework.h"
#include <assert.h>
#include <d3dcompiler.h>
#pragma comment(lib,"d3dcompiler")

struct Vertex {
	float x, y, z;
	COLORREF color;
};


Renderer::~Renderer() {
	SAFE_RELEASE(m_pDevice);
	SAFE_RELEASE(m_pDeviceContext);
	SAFE_RELEASE(m_pSwapChain);
	SAFE_RELEASE(m_pBackBufferRTV);
	SAFE_RELEASE(m_pIndexBuffer);
	SAFE_RELEASE(m_pVertexBuffer);
	SAFE_RELEASE(m_pVertexShader);
	SAFE_RELEASE(m_pPixelShader);
	SAFE_RELEASE(m_pInputLayout);
	SAFE_RELEASE(m_pRasterizerState);
	SAFE_RELEASE(m_pWorldMatrixBuffer);
	SAFE_RELEASE(m_pSceneMatrixBuffer);
#ifdef _DEBUG
	UINT flags = 0;
	if (m_pDevice) {
		flags = m_pDevice->GetCreationFlags();
	}
	if (flags & D3D11_CREATE_DEVICE_DEBUG) {
		ID3D11Debug* pDebug = nullptr;
		m_pDevice->QueryInterface(__uuidof(ID3D11Debug), reinterpret_cast<void**>(&pDebug));
		if (pDebug) {
			pDebug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
			pDebug->Release();
		}
	}
#endif
}

void Renderer::RenderFrame() {
	m_pDeviceContext->ClearState();
	ID3D11RenderTargetView* views[] = { m_pBackBufferRTV };
	m_pDeviceContext->OMSetRenderTargets(1, views, nullptr);
	static const FLOAT BackColor[4] = { 0.5f, 0.5f, 0.0f, 1.0f };
	m_pDeviceContext->ClearRenderTargetView(m_pBackBufferRTV, BackColor);

	D3D11_VIEWPORT viewport;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = (FLOAT)m_width;
	viewport.Height = (FLOAT)m_height;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	m_pDeviceContext->RSSetViewports(1, &viewport);

	D3D11_RECT rect;
	rect.left = 0;
	rect.right = m_width;
	rect.top = 0;
	rect.bottom = m_height;

	m_pDeviceContext->RSSetScissorRects(1, &rect);
	m_pDeviceContext->RSSetState(m_pRasterizerState);
	m_pDeviceContext->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

	ID3D11Buffer* vBuffer[] = { m_pVertexBuffer };
	UINT strides[] = { 16 };
	UINT offsets[] = { 0 };

	m_pDeviceContext->IASetVertexBuffers(0, 1, vBuffer, strides, offsets);
	m_pDeviceContext->VSSetConstantBuffers(0, 1, &m_pWorldMatrixBuffer);
	m_pDeviceContext->VSSetConstantBuffers(1, 1, &m_pSceneMatrixBuffer);
	m_pDeviceContext->IASetInputLayout(m_pInputLayout);
	m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_pDeviceContext->VSSetShader(m_pVertexShader, nullptr, 0);
	m_pDeviceContext->PSSetShader(m_pPixelShader, nullptr, 0);
	m_pDeviceContext->DrawIndexed(36, 0, 0);
	HRESULT result = m_pSwapChain->Present(1, 0);
	assert(SUCCEEDED(result));

}

bool Renderer::Resize(UINT width, UINT height) {
	if (width != m_width || height != m_height) {
		SAFE_RELEASE(m_pBackBufferRTV);
		HRESULT result = m_pSwapChain->ResizeBuffers(2, width, height, DXGI_FORMAT_R8G8B8A8_UNORM, 0);
		assert(SUCCEEDED(result));
		if (SUCCEEDED(result)) {
			m_width = width;
			m_height = height;

			result = SetupBackBuffer();
			m_pInput->resize(width, height);
		}
		return SUCCEEDED(result);
	}
	return true;
}

HRESULT Renderer::SetupBackBuffer() {
	ID3D11Texture2D* pBackBuffer = NULL;
	HRESULT result = m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
	assert(SUCCEEDED(result));
	if (SUCCEEDED(result)) {
		result = m_pDevice->CreateRenderTargetView(pBackBuffer, nullptr, &m_pBackBufferRTV);
		assert(SUCCEEDED(result));
		SAFE_RELEASE(pBackBuffer);
	}
	return result;
}

bool Renderer::Init(HWND hWnd, Camera* pCamera, Input* pInput) {
	m_pCamera = pCamera;
	m_pInput = pInput;
	HRESULT result;
	IDXGIFactory* pFactory = nullptr;
	result = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&pFactory);
	IDXGIAdapter* pSelectedAdapter = NULL;
	if (SUCCEEDED(result)) {
		IDXGIAdapter* pAdapter = NULL;
		UINT adapterIdx = 0;
		while (SUCCEEDED(pFactory->EnumAdapters(adapterIdx, &pAdapter))) {
			DXGI_ADAPTER_DESC desc;
			pAdapter->GetDesc(&desc);
			if (wcscmp(desc.Description, L"Microsoft Basic Render Driver") != 0) {
				pSelectedAdapter = pAdapter;
				break;
			}
			pAdapter->Release();
			adapterIdx++;
		}
	}
	assert(pSelectedAdapter != NULL);

	D3D_FEATURE_LEVEL level;
	D3D_FEATURE_LEVEL levels[] = { D3D_FEATURE_LEVEL_11_0 };
	if (SUCCEEDED(result)) {
		UINT flags = 0;
#ifdef _DEBUG
		flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
		result = D3D11CreateDevice(pSelectedAdapter, D3D_DRIVER_TYPE_UNKNOWN, NULL,
			flags, levels, 1, D3D11_SDK_VERSION, &m_pDevice, &level, &m_pDeviceContext);
		assert(level == D3D_FEATURE_LEVEL_11_0);
		assert(SUCCEEDED(result));
	}
	if (SUCCEEDED(result)) {
		RECT rc;
		GetClientRect(hWnd, &rc);
		m_width = rc.right - rc.left;
		m_height = rc.bottom - rc.top;

		DXGI_SWAP_CHAIN_DESC swapChainDesc = { 0 };
		swapChainDesc.BufferCount = 2;
		swapChainDesc.BufferDesc.Width = m_width;
		swapChainDesc.BufferDesc.Height = m_height;
		swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
		swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.OutputWindow = hWnd;
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.SampleDesc.Quality = 0;
		swapChainDesc.Windowed = true;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapChainDesc.Flags = 0;

		result = pFactory->CreateSwapChain(m_pDevice, &swapChainDesc, &m_pSwapChain);
		assert(SUCCEEDED(result));
	}
	if (SUCCEEDED(result)) {
		result = SetupBackBuffer();
	}
	if (SUCCEEDED(result)) {
		result = InitScene();
	}
	SAFE_RELEASE(pSelectedAdapter);
	SAFE_RELEASE(pFactory);

	if (SUCCEEDED(result)) {
		if (!m_pCamera)
			result = S_FALSE;
	}

	if (SUCCEEDED(result)) {
		if (!m_pInput)
			result = S_FALSE;
	}


	return SUCCEEDED(result);
}


HRESULT Renderer::InitScene() {
	HRESULT result;
	{
		/*vertices*/
		static const Vertex Vertices[] = {
			{ -1.0f, 1.0f, -1.0f, RGB(255, 0, 255) },
			{ 1.0f, 1.0f, -1.0f, RGB(0, 255, 255) },
			{ 1.0f, 1.0f, 1.0f, RGB(255, 255, 0) },
			{ -1.0f, 1.0f, 1.0f, RGB(0, 255, 0) },
			{ -1.0f, -1.0f, -1.0f, RGB(255, 0, 0) },
			{ 1.0f, -1.0f, -1.0f, RGB(0, 0, 255) },
			{ 1.0f, -1.0f, 1.0f, RGB(255, 255, 255) },
			{ -1.0f, -1.0f, 1.0f, RGB(255, 128, 255) }
		};
		D3D11_BUFFER_DESC desc = {};
		desc.ByteWidth = sizeof(Vertices);
		desc.Usage = D3D11_USAGE_IMMUTABLE;
		desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;
		desc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA data;
		data.pSysMem = &Vertices;
		data.SysMemPitch = sizeof(Vertices);
		data.SysMemSlicePitch = 0;

		result = m_pDevice->CreateBuffer(&desc, &data, &m_pVertexBuffer);
		assert(SUCCEEDED(result));
	}
	{
		/*indices*/
		static const USHORT Indices[] = {
			3,1,0,
			2,1,3,

			0,5,4,
			1,5,0,

			3,4,7,
			0,4,3,

			1,6,5,
			2,6,1,

			2,7,6,
			3,7,2,

			6,4,5,
			7,4,6,
		};

		D3D11_BUFFER_DESC desc = {};
		desc.ByteWidth = sizeof(Indices);
		desc.Usage = D3D11_USAGE_IMMUTABLE;
		desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;
		desc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA data;
		data.pSysMem = &Indices;
		data.SysMemPitch = sizeof(Indices);
		data.SysMemSlicePitch = 0;

		result = m_pDevice->CreateBuffer(&desc, &data, &m_pIndexBuffer);
		assert(SUCCEEDED(result));
	}

	ID3D10Blob* vShaderBuffer = nullptr;
	ID3D10Blob* pShaderBuffer = nullptr;

	int flags = 0;
#ifdef _DEBUG
	flags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif


	/*compile shaders*/
	result = D3DCompileFromFile(L"vertex-shader.hlsl", NULL, NULL, "vs", "vs_5_0", flags, 0, &vShaderBuffer, NULL);
	assert(SUCCEEDED(result));
	result = m_pDevice->CreateVertexShader(vShaderBuffer->GetBufferPointer(), vShaderBuffer->GetBufferSize(), NULL, &m_pVertexShader);
	assert(SUCCEEDED(result));

	result = D3DCompileFromFile(L"pixel-shader.hlsl", NULL, NULL, "ps", "ps_5_0", flags, 0, &pShaderBuffer, NULL);
	assert(SUCCEEDED(result));
	result = m_pDevice->CreatePixelShader(pShaderBuffer->GetBufferPointer(), pShaderBuffer->GetBufferSize(), NULL, &m_pPixelShader);
	assert(SUCCEEDED(result));

	/*input layout*/
	static const D3D11_INPUT_ELEMENT_DESC InputDesc[] = {
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"COLOR", 0,  DXGI_FORMAT_R8G8B8A8_UNORM, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	int numElements = sizeof(InputDesc) / sizeof(InputDesc[0]);
	result = m_pDevice->CreateInputLayout(InputDesc, numElements, vShaderBuffer->GetBufferPointer(), vShaderBuffer->GetBufferSize(), &m_pInputLayout);
	assert(SUCCEEDED(result));

	SAFE_RELEASE(vShaderBuffer);
	SAFE_RELEASE(pShaderBuffer);

	{
		D3D11_BUFFER_DESC desc = {};
		desc.ByteWidth = sizeof(WorldMatrixBuffer);
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;
		desc.StructureByteStride = 0;

		WorldMatrixBuffer worldMatrixBuffer;
		worldMatrixBuffer.mWorldMatrix = DirectX::XMMatrixIdentity();

		D3D11_SUBRESOURCE_DATA data;
		data.pSysMem = &worldMatrixBuffer;
		data.SysMemPitch = sizeof(worldMatrixBuffer);
		data.SysMemSlicePitch = 0;

		result = m_pDevice->CreateBuffer(&desc, &data, &m_pWorldMatrixBuffer);
		assert(SUCCEEDED(result));
	}
	{
		D3D11_BUFFER_DESC desc = {};
		desc.ByteWidth = sizeof(SceneMatrixBuffer);
		desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		desc.MiscFlags = 0;
		desc.StructureByteStride = 0;

		result = m_pDevice->CreateBuffer(&desc, nullptr, &m_pSceneMatrixBuffer);
		assert(SUCCEEDED(result));
	}
	{
		D3D11_RASTERIZER_DESC desc = {};
		desc.AntialiasedLineEnable = false;
		desc.FillMode = D3D11_FILL_SOLID;
		desc.CullMode = D3D11_CULL_BACK;
		desc.DepthBias = 0;
		desc.DepthBiasClamp = 0.0f;
		desc.FrontCounterClockwise = false;
		desc.DepthClipEnable = true;
		desc.ScissorEnable = false;
		desc.MultisampleEnable = false;
		desc.SlopeScaledDepthBias = 0.0f;

		result = m_pDevice->CreateRasterizerState(&desc, &m_pRasterizerState);
		assert(SUCCEEDED(result));
	}


	return result;
}

void Renderer::InputMovement() {
	XMFLOAT3 mouseMove = m_pInput->getMouseState();
	m_pCamera->getMouseState(mouseMove.x, mouseMove.y, mouseMove.z);
}

bool Renderer::GetState() {
	HRESULT hr = S_OK;
	m_pCamera->getState();
	m_pInput->getState();

	InputMovement();

	static float t = 0.0f;
	static ULONGLONG timeStart = 0;
	ULONGLONG timeCur = GetTickCount64();
	if (timeStart == 0) {
		timeStart = timeCur;
	}
	t = (timeCur - timeStart) / 500.0f;

	WorldMatrixBuffer wmb;
	wmb.mWorldMatrix = XMMatrixRotationY(t);
	m_pDeviceContext->UpdateSubresource(m_pWorldMatrixBuffer, 0, nullptr, &wmb, 0, 0);

	XMMATRIX mView;
	m_pCamera->getView(mView);

	XMMATRIX mProjection = XMMatrixPerspectiveFovLH(
		XM_PIDIV2,
		m_width / (FLOAT)m_height,
		0.01f, 100.0f);

	D3D11_MAPPED_SUBRESOURCE subresource;
	hr = m_pDeviceContext->Map(m_pSceneMatrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &subresource);
	assert(SUCCEEDED(hr));
	if (SUCCEEDED(hr)) {
		SceneMatrixBuffer& sceneBuffer = *reinterpret_cast<SceneMatrixBuffer*>(subresource.pData);
		sceneBuffer.mViewProjectionMatrix = XMMatrixMultiply(mView, mProjection);
		m_pDeviceContext->Unmap(m_pSceneMatrixBuffer, 0);
	}

	return SUCCEEDED(hr);
}

