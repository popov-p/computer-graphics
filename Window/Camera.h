#pragma once

#include <d3d11.h>
#include <directxmath.h>

using namespace DirectX;

class Camera {
private:
	XMMATRIX m_viewMatrix;
	XMFLOAT3 m_poi;

	float m_dist;
	float m_phi;
	float m_theta;
public:
	Camera() {
		m_poi = XMFLOAT3(0.0f, 0.0f, 0.0f);
		m_dist = 3.0f;
		m_phi = -XM_PIDIV4;
		m_theta = -XM_PIDIV4;
		m_viewMatrix = XMMATRIX();
	};

	void getState();
	void getMouseState(float dx, float dy, float wheel);
	void getView(XMMATRIX& viewMatrix) { viewMatrix = m_viewMatrix; };
};